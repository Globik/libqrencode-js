/*
var addon=require("./build/Release/test_async");
// colling a method Test - first time it's OK

addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);// 10
})

// but right after it it's not OK, it's crashed, but why??? Race condition???

addon.Test(5,{type:'start'},function(err, val){
console.log(err);
console.log(val);// 10
})

*/

#include <node_api.h>
#include "common.h"
#if defined _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#define nullptr ((void*)0)
typedef struct{
int32_t _input;
int32_t _output;
napi_ref _callback;
napi_async_work _request;
}carrier;
carrier the_carrier;

void Execute(napi_env env,void*data){
carrier* c=(carrier*)data;
	if(c !=&the_carrier){napi_throw_type_error(env,nullptr,"wrong data to execute.\n");return;}
	#if defined _WIN32
	Sleep(1000);
	#else
	sleep(1);
	#endif
	c->_output=c->_input*2;
}
void Complete(napi_env env,napi_status status, void* data){
    carrier* c=(carrier*)data;
	
	if(c !=&the_carrier){
	napi_throw_type_error(env,nullptr,"wrong data parameter to Complete");
		return;
	}
	
	if(status !=napi_ok){
	napi_throw_type_error(env,nullptr,"execute callback failed.");
		return;
	}
    //napi_handle_scope scope;
	napi_value argv[2];
	//napi_open_handle_scope(env,&scope);
	NAPI_CALL_RETURN_VOID(env,napi_get_null(env,&argv[0]));
	

	NAPI_CALL_RETURN_VOID(env, napi_create_int32(env,c->_output,&argv[1]));
	
	napi_value callback;
	NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env,c->_callback,&callback));
	napi_value global;
	NAPI_CALL_RETURN_VOID(env, napi_get_global(env,&global));
	napi_value result;
	NAPI_CALL_RETURN_VOID(env, napi_call_function(env,global,callback,2,argv,&result));
	NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env,c->_callback));
	NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env,c->_request));
	//napi_close_handle_scope(env,scope);
}

napi_value Test(napi_env env,napi_callback_info info){
	//napi_handle_scope scope;
size_t argc=3;
	
	napi_value argv[3];
	napi_value _this;
	napi_value resource_name;
	void* data;
	//napi_open_handle_scope(env,&scope);
	
	NAPI_CALL(env,napi_get_cb_info(env,info,&argc,argv,&_this,&data));
	
	NAPI_ASSERT(env,argc >=3,"not enough arguments, expected 3?.");
	
	
	the_carrier._output=0;
	NAPI_CALL(env,napi_get_value_int32(env,argv[0],&the_carrier._input));
	
	
	NAPI_CALL(env, napi_create_reference(env,argv[2],1,&the_carrier._callback));
	
	NAPI_CALL(env, napi_create_string_utf8(env,"TestResource",NAPI_AUTO_LENGTH,&resource_name));
	
	NAPI_CALL(env, napi_create_async_work(env,argv[1],resource_name,Execute,Complete,&the_carrier,&the_carrier._request));
	
	NAPI_CALL(env, napi_queue_async_work(env,the_carrier._request));
	
	//napi_close_handle_scope(env,scope);
	return nullptr;
	}


napi_value Init(napi_env env,napi_value exports){

napi_property_descriptor properties[]={
	DECLARE_NAPI_PROPERTY("Test",Test),
};
NAPI_CALL(env, napi_define_properties(env,exports,sizeof(properties)/sizeof(*properties),properties));
return exports;
}
//NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
NAPI_MODULE(addon,Init);