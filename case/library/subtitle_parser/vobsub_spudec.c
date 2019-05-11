#include "vobsub_spudec.h"
#include "subtitle_parser.h"




static unsigned int get_be16(const unsigned char *p)
{
	return (p[0] << 8) + p[1];
}

static unsigned int get_be24(const unsigned char *p)
{
	return (get_be16(p) << 8) + p[2];
}



static unsigned char get_nibble(packet_t *packet)
{
	unsigned char nib;
	unsigned int *nibblep = packet->current_nibble + packet->deinterlace_oddness;
	if (*nibblep / 2 >= packet->control_start) {
		printf("SPUdec: ERROR: get_nibble past end of packet\n");
		return 0;
	}
	nib = packet->packet[*nibblep / 2];
	if (*nibblep % 2)
		nib &= 0xf;
	else
		nib >>= 4;
	++*nibblep;
	return nib;
}


static void next_line(packet_t *packet)
{
	if (packet->current_nibble[packet->deinterlace_oddness] % 2)
		packet->current_nibble[packet->deinterlace_oddness]++;
	packet->deinterlace_oddness = (packet->deinterlace_oddness + 1) % 2;
}


static int mkalpha(int i)
{
/* In mplayer's alpha planes, 0 is transparent, then 1 is nearly
	opaque upto 255 which is transparent */
	switch (i) {
		case 0xf:
			return 1;
		case 0:
			return 0;
		default:
			return (0xf - i) << 4;
	}
}


/* Cut the sub to visible part */
static void spudec_cut_image(spudec_handle_t *pthis,parser_info_t *parser_info)
{
	unsigned int fy, ly;
	unsigned int first_y, last_y;
	unsigned char *image;
	unsigned char *aimage;

	if (pthis->stride == 0 || pthis->height == 0) {
		return;
	}

	for (fy = 0; fy < pthis->image_size && !pthis->aimage[fy]; fy++);
	for (ly = pthis->stride * pthis->height-1; ly && !pthis->aimage[ly]; ly--);
	first_y = fy / pthis->stride;
	last_y = ly / pthis->stride;
	//printf("first_y: %d, last_y: %d\n", first_y, last_y);
	pthis->start_row += first_y;

	// Some subtitles trigger this condition
	if (last_y + 1 > first_y ) {
		pthis->height = last_y - first_y +1;
	} else {
		pthis->height = 0;
		pthis->image_size = 0;
		return;
	}

	//  printf("new h %d new start %d (sz %d st %d)---\n\n", this->height, this->start_row, this->image_size, this->stride);

	image = parser_info->fops.malloc(2 * pthis->stride * pthis->height);
	if(image){
		pthis->image_size = pthis->stride * pthis->height;
		aimage = image + pthis->image_size;
		memcpy(image, pthis->image + pthis->stride * first_y, pthis->image_size);
		memcpy(aimage, pthis->aimage + pthis->stride * first_y, pthis->image_size);
		parser_info->fops.free(pthis->image);
		pthis->image = image;
		pthis->aimage = aimage;
	} else {
		printf("Fatal: update_spu: malloc requested %d bytes\n", 2 * pthis->stride * pthis->height);
	}
}

static void spudec_queue_packet(spudec_handle_t *pthis, packet_t *packet)
{
	if (pthis->queue_head == NULL)
		pthis->queue_head = packet;
	else
		pthis->queue_tail->next = packet;
	pthis->queue_tail = packet;
}

static packet_t *spudec_dequeue_packet(spudec_handle_t *pthis)
{
	packet_t *retval = pthis->queue_head;

	pthis->queue_head = retval->next;
	if (pthis->queue_head == NULL)
		pthis->queue_tail = NULL;
	return retval;
}

static void spudec_free_packet(packet_t *packet,parser_info_t *parser_info)
{
	if (packet->packet != NULL)
		parser_info->fops.free(packet->packet);
	parser_info->fops.free(packet);
}


/* get palette custom color, width, height from .idx file */
//void *spudec_new_scaled_vobsub(unsigned int *palette, unsigned int *cuspal, unsigned int custom, unsigned int frame_width, unsigned int frame_height)
//void *spudec_new_scaled_vobsub(vobsub_t *vob,parser_info_t *parser_info)
void *spudec_new_scaled_vobsub(unsigned int *palette, unsigned int *cuspal, unsigned int custom, unsigned int frame_width, unsigned int frame_height,parser_info_t *parser_info)
{
	spudec_handle_t *hspudec = (spudec_handle_t*)(parser_info->fops.malloc(sizeof(spudec_handle_t)));
	if (hspudec){
		//(fprintf(stderr,"VobSub Custom Palette: %d,%d,%d,%d", this->cuspal[0], this->cuspal[1], this->cuspal[2],this->cuspal[3]);
		hspudec->packet = NULL;
		hspudec->image = NULL;
		hspudec->scaled_image = NULL;
		/* XXX Although the video frame is some size, the SPU frame is
		always maximum size i.e. 720 wide and 576 or 480 high */
		hspudec->orig_frame_width = 720;
		hspudec->orig_frame_height = (frame_height == 480 || frame_height == 240) ? 480 : 576;
		hspudec->custom = custom;
		// set up palette:
		hspudec->auto_palette = 1;
		if (palette){
			memcpy(hspudec->global_palette,palette, sizeof(hspudec->global_palette));
			hspudec->auto_palette = 0;
		}
		hspudec->custom = custom;
		if (custom && cuspal) {
			memcpy(hspudec->cuspal, cuspal, sizeof(hspudec->cuspal));
			hspudec->auto_palette = 0;
		}
		// forced subtitles default: show all subtitles
		hspudec->forced_subs_only=0;
		hspudec->is_forced_sub=0;
		hspudec->output_buf=NULL;
		hspudec->output_w=0;
		hspudec->output_h =0;
		//hspudec->output_format = OUTPUT_FORMAT_RGB565;
		//hspudec->output_format = OUTPUT_FORMAT_OSD2BIT;
		hspudec->output_format = OUTPUT_FORMAT_OSD8BIT;
	}
	else
		printf("FATAL: spudec_init: malloc failed");
	return hspudec;
}


static void __print_packet(packet_t *packet)
{
	int i;
	printf("$$$$Packet Start$$$$$$\n");
	printf("Packet=0x%x\n",packet->packet);
	printf("Palette:");
	for(i=0;i<4;i++)
		printf("[%d]=0x%x  ",i,packet->palette[i]);
	printf("\n");

	printf("alpha:");
	for(i=0;i<4;i++)
		printf("[%d]=0x%x  ",i,packet->alpha[i]);
	printf("\n");

	printf("Control_start=0x%x,nibble[0]=0x%x,nibble[1]=0x%x,",packet->control_start,packet->current_nibble[0],\
		packet->current_nibble[1]);

	printf("deinterlace_oddness=%d,start_col=%d,end_col=%d,start_row=%d,end_row=%d,width=%d,height=%d,stride=%d\n",
		packet->deinterlace_oddness,packet->start_col,packet->end_col,packet->start_row,packet->end_row,packet->width,\
		packet->height,packet->stride);

	printf("start_pts=0x%x,end_pts=0x%x,Next=0x%x\n",packet->start_pts,packet->end_pts,packet->next);

	printf("$$$$Packet END$$$$$$\n");

}

static void __print_spudec_handle(spudec_handle_t *pthis)
{
	int i=0;
	packet_t *tmppacket;
	printf("###########Start###############\n");
	printf("Global_Palette:");
	for(i=0;i<16;i++)
		printf("[%d]=0x%x",i,pthis->global_palette[i]);
	printf("\n");
	printf("Orig_Frame_W=%d,Orig_Frame_H=%d\n",pthis->orig_frame_width,pthis->orig_frame_height);
	printf("Packet Information:Packet=0x%x,packet_reserve=0x%x,packet_offset=0x%x,packet_size=0x%x\n",pthis->packet,\
		pthis->packet_reserve,pthis->packet_offset,pthis->packet_size);
	printf("Palette:");
	for(i=0;i<4;i++){
		printf("[%d]=0x%x",i,pthis->palette[i]);
	}
	printf("\n");

	printf("alpha:");
	for(i=0;i<4;i++){
		printf("[%d]=0x%x",i,pthis->alpha[i]);
	}
	printf("\n");

	printf("cuspal:");
	for(i=0;i<4;i++){
		printf("[%d]=0x%x",i,pthis->cuspal[i]);
	}
	printf("\n");
	printf("Custom=%d,now_pts=0x%x,start_pts=0x%x,end_pts=0x%x\n",pthis->custom,pthis->now_pts,pthis->start_pts,\
		pthis->end_pts);

	printf("start_col=0x%x,end_col=0x%x,start_row=0x%x,end_row=0x%x\n",pthis->start_col,pthis->end_col,pthis->start_row,pthis->end_row);
	printf("width=%d,height=%d,stride=%d,ImageSize=0x%x\n",pthis->width,pthis->height,pthis->stride,pthis->image_size);
	printf("Image=0x%x,aImage=0x%x,scaled_frame_width=%d,scaled_frame_height=%d\n",pthis->image,pthis->aimage,\
		pthis->scaled_frame_width,pthis->scaled_frame_height);
	printf("scaled_start_col=%d,scaled_start_row=%d,scaled_width=%d,scaled_height=%d,scaled_stride=%d\n",
		pthis->scaled_start_col,pthis->scaled_start_row,pthis->scaled_width,pthis->scaled_height,pthis->scaled_stride);
	printf("scaled_image_size=0x%x,scaled_image=0x%x,scaled_aimage=0x%x\n",pthis->scaled_image_size,pthis->scaled_image,pthis->scaled_aimage);
	printf("auto_palette=%d,font_start_level=%d,forced_subs_only=%d,is_forced_sub=%d\n",pthis->auto_palette,pthis->font_start_level,
		pthis->font_start_level,pthis->is_forced_sub);


	if(pthis->queue_head!=NULL){
		printf("QQQQ Head\n");
		__print_packet(pthis->queue_head);
	}
	tmppacket = pthis->queue_tail;
	while(1){
		if(tmppacket!=NULL){
			__print_packet(tmppacket);
			tmppacket = tmppacket->next;
		}
		else
			break;
	}
	printf("###########End###############\n");
}

static void spudec_process_control(spudec_handle_t *pthis, unsigned int pts100,parser_info_t *parser_info)
{
	int a,b; /* Temporary vars */
	unsigned int date, type;
	unsigned int off;
	unsigned int start_off = 0;
	unsigned int next_off;
	unsigned int start_pts;
	unsigned int end_pts;
	unsigned int current_nibble[2];
	unsigned int control_start;
	unsigned int display = 0;
	unsigned int start_col = 0;
	unsigned int end_col = 0;
	unsigned int start_row = 0;
	unsigned int end_row = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int stride = 0;

	control_start = get_be16(pthis->packet + 2);
	next_off = control_start;
	while (start_off != next_off) {
		start_off = next_off;
		date = get_be16(pthis->packet + start_off) * 1024;
		next_off = get_be16(pthis->packet + start_off + 2);
		//printf("date=%d\n", date);
		off = start_off + 4;
		for (type = pthis->packet[off++]; type != 0xff; type = pthis->packet[off++]) {
			//printf("cmd=%d  ",type);
			switch(type) {
				case 0x00:
					/* Menu ID, 1 byte */
				//	printf("Menu ID\n");
					/* shouldn't a Menu ID type force display start? */
					start_pts = pts100 + date;
					end_pts = 0xFFFFFFFF;
					display = 1;
					pthis->is_forced_sub=~0; // current subtitle is forced
					break;
				case 0x01:
					/* Start display */
					
					start_pts = pts100 + date;
					end_pts = 0xffffffff;
					display = 1;
					pthis->is_forced_sub=0;
					//printf("Start display!SPts=0x%x\n",start_pts);
					break;
				case 0x02:
					/* Stop display */
					end_pts = pts100 + date+1;
					//printf("Stop display!EPts=0x%x\n",end_pts);
					break;
				case 0x03:
					/* Palette */
					pthis->palette[0] = pthis->packet[off] >> 4;
					pthis->palette[1] = pthis->packet[off] & 0xf;
					pthis->palette[2] = pthis->packet[off + 1] >> 4;
					pthis->palette[3] = pthis->packet[off + 1] & 0xf;
				//	printf("Palette %d, %d, %d, %d\n",pthis->palette[0], pthis->palette[1], pthis->palette[2], pthis->palette[3]);
					off+=2;
					break;
				case 0x04:
					/* Alpha */
					pthis->alpha[0] = pthis->packet[off] >> 4;
					pthis->alpha[1] = pthis->packet[off] & 0xf;
					pthis->alpha[2] = pthis->packet[off + 1] >> 4;
					pthis->alpha[3] = pthis->packet[off + 1] & 0xf;
				//	printf("Alpha %d, %d, %d, %d\n",pthis->alpha[0], pthis->alpha[1], pthis->alpha[2], pthis->alpha[3]);
					off+=2;
					break;
				case 0x05:
					/* Co-ords */
					a = get_be24(pthis->packet + off);
					b = get_be24(pthis->packet + off + 3);
					start_col = a >> 12;
					end_col = a & 0xfff;
					width = (end_col < start_col) ? 0 : end_col - start_col + 1;
					stride = (width + 7) & ~7; /* Kludge: draw_alpha needs width multiple of 8 */
					start_row = b >> 12;
					end_row = b & 0xfff;
					height = (end_row < start_row) ? 0 : end_row - start_row /* + 1 */;
				//	printf("Coords  col: %d - %d  row: %d - %d  (%dx%d)\n",start_col, end_col, start_row, end_row,width, height);
					off+=6;
					break;
				case 0x06:
					/* Graphic lines */
					current_nibble[0] = 2 * get_be16(pthis->packet + off);
					current_nibble[1] = 2 * get_be16(pthis->packet + off + 2);
			//		printf("Graphic offset 1: %d  offset 2: %d\n",current_nibble[0] / 2, current_nibble[1] / 2);
					off+=4;
					break;
				case 0xff:
					/* All done, bye-bye */
			//		printf("Done!\n");
					return;
					//	break;
				default:
					printf("spudec: Error determining control type 0x%02x.  Skipping %d bytes.\n",type, next_off - off);
					goto next_control;
			}
		}
next_control:
		if (display) {
			packet_t *packet = parser_info->fops.malloc(sizeof(packet_t));
			int i;
			packet->start_pts = start_pts;
			if (end_pts == 0xFFFFFFFF && start_off != next_off) {
				start_pts = pts100 + get_be16(pthis->packet + next_off) * 1024;
				packet->end_pts = start_pts - 1;
			} else
				packet->end_pts = end_pts;
			packet->current_nibble[0] = current_nibble[0];
			packet->current_nibble[1] = current_nibble[1];
			packet->start_row = start_row;
			packet->end_row = end_row;
			packet->start_col = start_col;
			packet->end_col = end_col;
			packet->width = width;
			packet->height = height;
			packet->stride = stride;
			packet->control_start = control_start;
			for (i=0; i<4; i++) {
				packet->alpha[i] = pthis->alpha[i];
				packet->palette[i] = pthis->palette[i];
			}
			packet->packet = parser_info->fops.malloc(pthis->packet_size);
			memcpy(packet->packet, pthis->packet, pthis->packet_size);
			//printf("P=0x%x,PackSize=0x%x,",packet->packet,pthis->packet_size);
			spudec_queue_packet(pthis, packet);
			//__print_spudec_handle(pthis);
		}
	}
}


static void spudec_decode(spudec_handle_t *pthis, unsigned int pts100,parser_info_t	*parser_info)
{	
	spudec_process_control(pthis, pts100,parser_info);
}



void spudec_assemble(void *pspudec,unsigned char *packet, unsigned int len, unsigned int pts100,parser_info_t *parser_info)
{
	vobsub_info_t *vobsub_info = (vobsub_info_t*)(parser_info->finfo);
	spudec_handle_t *spu = (spudec_handle_t*)vobsub_info->spudec;
	//  spudec_heartbeat(this, pts100);
	if (len < 2) {
		printf("SPUasm: packet too short\n");
		return;
	}
#if 0
	if ((spu->packet_pts + 10000) < pts100) {
		// [cb] too long since last fragment: force new packet
		spu->packet_offset = 0;
	}
#endif
	spu->packet_pts = pts100;
	if (spu->packet_offset == 0) {
		unsigned int len2 = get_be16(packet);
		// Start new fragment
		if (spu->packet_reserve < len2) {
			if (spu->packet != NULL)
				parser_info->fops.free(spu->packet);
			spu->packet = parser_info->fops.malloc(len2);
			spu->packet_reserve = spu->packet != NULL ? len2 : 0;
		}
		if (spu->packet != NULL) {
			spu->packet_size = len2;
			if (len > len2) {
				printf("SPUasm: invalid frag len / len2: %d / %d \n", len, len2);
				return;
			}
			memcpy(spu->packet, packet, len);
			spu->packet_offset = len;
			spu->packet_pts = pts100;
		}
	} else {
		// Continue current fragment
		if (spu->packet_size < spu->packet_offset + len){
			printf("SPUasm: invalid fragment\n");
			spu->packet_size = spu->packet_offset = 0;
			return;
		} else {
			memcpy(spu->packet + spu->packet_offset, packet, len);
			spu->packet_offset += len;
		}
	}
#if 1
	// check if we have a complete packet (unfortunatelly packet_size is bad
	// for some disks)
	// [cb] packet_size is padded to be even -> may be one byte too long
	if ((spu->packet_offset == spu->packet_size) ||
		((spu->packet_offset + 1) == spu->packet_size)){
		unsigned int x=0,y;
		while(x+4<=spu->packet_offset){
			y=get_be16(spu->packet+x+2); // next control pointer
			//printf("SPUtest: x=0x%x y=0x%x off=0x%x size=0x%x\n",x,y,spu->packet_offset,spu->packet_size);
			if(x>=4 && x==y){		// if it points to self - we're done!
				// we got it!
				//printf("SPUgot: off=%d  size=%d \n",spu->packet_offset,spu->packet_size);
				spudec_decode(spu, pts100,parser_info);
				spu->packet_offset = 0;
				break;
			}
			if(y<=x || y>=spu->packet_size){ // invalid?
				printf("SPUtest: broken packet!!!!! y=%d < x=%d\n",y,x);
				spu->packet_size = spu->packet_offset = 0;
				break;
			}
			x=y;
		}
		// [cb] packet is done; start new packet
		spu->packet_offset = 0;
	}
#else
	if (spu->packet_offset == spu->packet_size) {
		spudec_decode(spu, pts100,parser_info);
		spu->packet_offset = 0;
	}
#endif
}



/*
This function tries to create a usable palette.
It determines how many non-transparent colors are used, and assigns different
gray scale values to each color.
I tested it with four streams and even got something readable. Half of the
times I got black characters with white around and half the reverse.
*/
static void compute_palette(spudec_handle_t *pthis, packet_t *packet)
{
	int used[16],i,cused,start,step,color;

	memset(used, 0, sizeof(used));
	for (i=0; i<4; i++)
		if (packet->alpha[i]) /* !Transparent? */
			used[packet->palette[i]] = 1;
	for (cused=0, i=0; i<16; i++)
		if (used[i]) cused++;
	if (!cused) return;
	if (cused == 1) {
		start = 0x80;
		step = 0;
	} else {
		start = pthis->font_start_level;
		step = (0xF0-pthis->font_start_level)/(cused-1);
	}
	memset(used, 0, sizeof(used));
	for (i=0; i<4; i++) {
		color = packet->palette[i];
		if (packet->alpha[i] && !used[color]) { /* not assigned? */
			used[color] = 1;
			pthis->global_palette[color] = start<<16;
			start += step;
		}
	}
}



int spudec_draw(void *pthis,int (*draw_alpha)(spudec_handle_t *spu))
{
	spudec_handle_t *spu = (spudec_handle_t *)pthis;
	int is_image_draw=0;
	if (spu->start_pts <= spu->now_pts && spu->now_pts < spu->end_pts && spu->image)
	{
		is_image_draw = draw_alpha(spu);
		printf("<Draw OK>\n");
		spu->spu_changed = 0;
	}
	//printf("IS IMAGE DRAW=%d\n",is_image_draw);
	return is_image_draw;
}


int spudec_changed(void * pthis)
{
	spudec_handle_t * spu = (spudec_handle_t*)pthis;
	return (spu->spu_changed || spu->now_pts > spu->end_pts);
}

static void spudec_process_data(spudec_handle_t *pthis, packet_t *packet,parser_info_t *parser_info)
{
	unsigned int cmap[4], alpha[4];
	unsigned int i, x, y;
	
	pthis->scaled_frame_width = 0;
	pthis->scaled_frame_height = 0;
	pthis->start_col = packet->start_col;
	pthis->end_col = packet->end_col;
	pthis->start_row = packet->start_row;
	pthis->end_row = packet->end_row;
	pthis->height = packet->height;
	pthis->width = packet->width;
	pthis->stride = packet->stride;
	for (i = 0; i < 4; ++i) {
		alpha[i] = mkalpha(packet->alpha[i]);
		if (alpha[i] == 0)
			cmap[i] = 0;
		else if (pthis->custom){
			cmap[i] = ((pthis->cuspal[i] >> 16) & 0xff);
			if (cmap[i] + alpha[i] > 255)
				cmap[i] = 256 - alpha[i];
		}
		else {
			cmap[i] = ((pthis->global_palette[packet->palette[i]] >> 16) & 0xff);
			if (cmap[i] + alpha[i] > 255)
				cmap[i] = 256 - alpha[i];
		}
	}
	
	if (pthis->image_size < pthis->stride * pthis->height) {
		if (pthis->image != NULL) {
			parser_info->fops.free(pthis->image);
			pthis->image_size = 0;
		}
		pthis->image = parser_info->fops.malloc(2 * pthis->stride * pthis->height);
		if (pthis->image) {
			pthis->image_size = pthis->stride * pthis->height;
			pthis->aimage = pthis->image + pthis->image_size;
		}
	}
	if (pthis->image == NULL)
		return;
	
	/* Kludge: draw_alpha needs width multiple of 8. */
	if (pthis->width < pthis->stride)
		for (y = 0; y < pthis->height; ++y) {
			memset(pthis->aimage + y * pthis->stride + pthis->width, 0, pthis->stride - pthis->width);
			/* FIXME: Why is this one needed? */
			memset(pthis->image + y * pthis->stride + pthis->width, 0, pthis->stride - pthis->width);
		}
		
		i = packet->current_nibble[1];
		x = 0;
		y = 0;
		while (packet->current_nibble[0] < i
			&& packet->current_nibble[1] / 2 < packet->control_start
			&& y < pthis->height) {
			unsigned int len, color;
			unsigned int rle = 0;
			rle = get_nibble(packet);
			if (rle < 0x04) {
				rle = (rle << 4) | get_nibble(packet);
				if (rle < 0x10) {
					rle = (rle << 4) | get_nibble(packet);
					if (rle < 0x040) {
						rle = (rle << 4) | get_nibble(packet);
						if (rle < 0x0004)
							rle |= ((pthis->width - x) << 2);
					}
				}
			}
			color = 3 - (rle & 0x3);
			len = rle >> 2;
			if (len > pthis->width - x || len == 0)
				len = pthis->width - x;
			/* FIXME have to use palette and alpha map*/
			memset(pthis->image + y * pthis->stride + x, cmap[color], len);
			memset(pthis->aimage + y * pthis->stride + x, alpha[color], len);
			x += len;
			if (x >= pthis->width) {
				next_line(packet);
				x = 0;
				++y;
			}
		}
		spudec_cut_image(pthis,parser_info);
}


void spudec_heartbeat(void *pthis, unsigned int pts100,parser_info_t *parser_info)
{ 
	vobsub_info_t *vobsub_info = (vobsub_info_t*)(parser_info->finfo);
	spudec_handle_t *spu = (spudec_handle_t*)vobsub_info->spudec;
	spu->now_pts = pts100;
	
	while (spu->queue_head != NULL && pts100 >= spu->queue_head->start_pts) {
		packet_t *packet = spudec_dequeue_packet(spu);
		spu->start_pts = packet->start_pts;
		spu->end_pts = packet->end_pts;
		if (spu->auto_palette)
			compute_palette(spu, packet);
		spudec_process_data(spu, packet,parser_info);
		spudec_free_packet(packet,parser_info);
		spu->spu_changed = 1;
	}
}