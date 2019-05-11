import Actions.AudioDbEngine;

var ret:Number;
var nr:Number;
var i:Number;

ret = AudioDbEngine.Open("/mnt/udisk/AudioInfoDB.db");
if(ret == 0){
	///------------------- 
	///test for the album
	///-------------------
	nr = AudioDbEngine.getAlbumNumber();
	trace("total album is "+nr);
	for(i=0;i<nr;i++){
		trace("the "+i+"th album --> "+AudioDbEngine.getAlbumInfo(i));
	}
	
	/// in this test we know there is an album named "ALBUM10"
	nr = AudioDbEngine.sortAlbum("ALBUM10",AudioDbEngine.DBAUDIO_SORT_BY_NAME);
	for(i=0;i<nr;i++){
		trace(AudioDbEngine.getSortedAlbumItemInfo("ALBUM10",i));
	}
	
	///------------------- 
	///test for the artist
	///-------------------
	nr = AudioDbEngine.getArtistNumber();
	trace("total artist is "+nr);
	for(i=0;i<nr;i++){
		trace("the "+i+"th artist --> "+AudioDbEngine.getArtistInfo(i));
	}
	
	/// in this test we know there is an album named "Artist10"
	nr = AudioDbEngine.sortArtist("Artist10",AudioDbEngine.DBAUDIO_SORT_BY_NAME);
	for(i=0;i<nr;i++){
		trace(AudioDbEngine.getSortedArtistItemInfo("Artist10",i));
	}
	
	
	///------------------- 
	///test for the genre
	///-------------------
	nr = AudioDbEngine.getGenreNumber();
	trace("total gnere is "+nr);
	for(i=0;i<nr;i++){
		trace("the "+i+"th gnere --> "+AudioDbEngine.getGenreInfo(i));
	}
	
	/// in this test we know there is an genre named "POP"
	nr = AudioDbEngine.sortGenre("POP",AudioDbEngine.DBAUDIO_SORT_BY_NAME);
	for(i=0;i<nr;i++){
		trace(AudioDbEngine.getSortedGenreItemInfo("POP",i));
	}
	
	
	///------------------- 
	///test for the song
	///-------------------
	nr = AudioDbEngine.getSongNumber();
	trace("total song is "+nr);
	
	nr = AudioDbEngine.sortSong(AudioDbEngine.DBAUDIO_SORT_BY_NAME);
	for(i=0;i<nr;i++){
		trace(AudioDbEngine.getSortedSongItemInfo(i));
	}
	
	AudioDbEngine.Close();
}
else{
	trace("open database error");
}