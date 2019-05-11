/*
 *  fs/partitions/msdos.h
 */

#define MSDOS_LABEL_MAGIC		0xAA55

int msdos_partition(struct parsed_partitions *state, struct block_device *bdev);

/* Hack for Actions-MicroEltronics native partitioning ... */
int amflash_partition(struct parsed_partitions *state, struct block_device *bdev);

