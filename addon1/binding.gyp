{
  "targets": [
    {
      "target_name": "asyncAddon",
      "sources": [ "async-addon.cc" ],
	  "include_dirs":["/home/globik/libqrencode"],
	  "link_settings":{
	  "libraries":["-lpng","-L/home/globik/libqrencode-js/addon1/deps/libqrencode/build/Release"],
	  "ldflags":["-Wl,-rpath,/home/globik/libqrencode-js/addon1/deps/libqrencode/build/Release"]
	  },
      "libraries":["-L /home/globik/libqrencode/.libs/libqrencode", "-lqrencode", "-lpng"],
	# "libraries":["-L/home/globik/libqrencode-js/addon1/deps/libqrencode/build/Release/qrenc", "-lpng"],
	 #"libraries":["-lpng"],
	  #/home/globik/libqrencode-js/addon1/deps/libqrencode/build/Release/qrenc.so
      "defines":["HAVE_PNG"]
    }
  ]
}