#{"targets":[{"target_name":"qrencode-js","sources":["simple_async.c"],"cflags":["-std=c99","-O3",],
#"include_dirs":["/home/globik/libqrencode"],
#"link_settings":{"libraries":["-lpng", "-L/home/globik/libqrencode-js/addon1/deps/libqrencode/build/Release","-lqrenca"],},
#"libraries":["-lpng"],"defines":["HAVE_PNG"]}]}

# ldd build/Release/qrencode-js.node
{"targets":[{"target_name":"qrencode-js","sources":["simple_async.c"],"cflags":["-std=c99","-O3"
],
"dependencies":[
"../addon1/deps/libqrencode/qrencode.qyp:liblibqrenca"
],
"sources":["simple_async.c"],
"cflags":["-std=c99","-O3"],
"include_dirs":["../addon1/deps/libqrencode"],
"libraries":["-lpng",
#"-L/home/globik/libqrencode-js/simple_async/build/Release -lqrenca"
#"-L/home/globik/libqrencode-js/simple_async/build/Release/libqrenca.a"
"-L./build/Release/libqrenca.a"
],
"defines":["HAVE_PNG"]
} 
]
}
# node-gyp rebuild