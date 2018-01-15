{
  "targets": [
 #   {
  #    "target_name": "liblibqrenc",
#  "type":"static_library",
 #     "sources": [ "bitstream.c","mask.c","qrencode.c","qrspec.c","mmask.c","rsecc.c","mqrspec.c","qrinput.c","split.c"],
#	  "include_dirs":["."],
      
  #    "defines":["HAVE_PNG","STATIC_IN_RELEASE=static","MAJOR_VERSION=4","MICRO_VERSION=0","MINOR_VERSION=0","VERSION=\"4.0.0\""]
  #  },
	 {
      "target_name": "liblibqrenca",
	  "type":"shared_library",
      "sources": [ "bitstream.c","mask.c","qrencode.c","qrspec.c","mmask.c","rsecc.c","mqrspec.c","qrinput.c","split.c"],
	  "include_dirs":["."],
      
      "defines":["HAVE_PNG","STATIC_IN_RELEASE=static","MAJOR_VERSION=4","MICRO_VERSION=0","MINOR_VERSION=0","VERSION=\"4.0.0\""]
    }
  ]
}