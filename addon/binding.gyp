{
  "targets": [
    {
      "target_name": "asyncAddon",
      "sources": [ "async-addon.cc" ],
	  "include_dirs":["/home/globik/libqrencode"],
      "libraries":["-L /home/globik/libqrencode/.libs/libqrencode", "-lqrencode", "-lpng"],
      "defines":["HAVE_PNG"]
    }
  ]
}