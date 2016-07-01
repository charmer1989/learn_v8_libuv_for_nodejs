#include <node.h>
#include <uv.h>

using namespace v8;

struct Work {
  uv_timer_t  timer_req;
  Persistent<Function> pcb;
};

uv_loop_t *loop;

void do_cb(uv_timer_t *req) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);

  Work *work = static_cast<Work *>(req->data);

  const unsigned argc = 2;
  Local<Value> argv[argc] = {Undefined(isolate), String::NewFromUtf8(isolate, "hello world") };
  Local<Function> fn = Local<Function>::New(isolate, work->pcb);
  fn -> Call(isolate->GetCurrentContext()->Global(), argc, argv);

  work->pcb.Reset();
  delete work;
}

void RunCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }
  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments first arg must be number")));
    return;
  }
  if (!args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments second arg must be function")));
    return;
  }

  Work * work = new Work();
  work->timer_req.data = work;
  work->pcb.Reset(isolate, Local<Function>::Cast(args[1]));

  uv_timer_init(loop, &work->timer_req);
  uv_timer_start(&work->timer_req, do_cb, args[0]->NumberValue(), 0);
}

void Init(Handle<Object> exports, Handle<Object> module) {
  loop = uv_default_loop();
  uv_run(loop, UV_RUN_DEFAULT);
  NODE_SET_METHOD(module, "exports", RunCallback);
}

NODE_MODULE(delaycb, Init)
