#include <node.h>
#include <uv.h>

using namespace v8;

// 每一个任务都要初始化一个Work对象
struct Work {
  uv_timer_t  timer_req;
  Persistent<Function> pcb;
};

// 事件循环根
uv_loop_t *loop;

// 到时间执行回掉的函数
void do_cb(uv_timer_t *req) {
  // 新版本v8都要这个
  Isolate* isolate = Isolate::GetCurrent();
  // 这貌似是v8进行内存管理的东西??? 执行js的函数必须要在这个之下
  HandleScope handleScope(isolate);
  // 解析libuv传进来的对象
  Work *work = static_cast<Work *>(req->data);
  // 构造调用js函数的参数列表
  const unsigned argc = 2;
  Local<Value> argv[argc] = {Undefined(isolate), String::NewFromUtf8(isolate, "hello world") };
  // 调用js传来的回掉函数
  Local<Function> fn = Local<Function>::New(isolate, work->pcb);
  fn -> Call(isolate->GetCurrentContext()->Global(), argc, argv);
  // 清理内存
  work->pcb.Reset();
  delete work;
}

// 这个是导出给js的函数
void RunCallback(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  // 参数验证
  if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }
  if (!args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments second arg must be function")));
    return;
  }
  if (!args[0]->IsNumber()) {
    const unsigned argc = 1;
    Local<Value> argv[argc] = {String::NewFromUtf8(isolate, "wrong type of argument") };
    Local<Function>::Cast(args[1])-> Call(isolate->GetCurrentContext()->Global(), argc, argv);
    return;
  }
  // 初始化一个work
  Work * work = new Work();
  // data字段记录引用, 异步回掉的时候通过他读work
  work->timer_req.data = work;
  // 将回调函数写入work
  work->pcb.Reset(isolate, Local<Function>::Cast(args[1]));
  // 初始化计时器
  uv_timer_init(loop, &work->timer_req);
  // 执行计时任务
  uv_timer_start(&work->timer_req, do_cb, args[0]->NumberValue(), 0);
}

void Init(Handle<Object> exports, Handle<Object> module) {
  loop = uv_default_loop();
  uv_run(loop, UV_RUN_DEFAULT);
  NODE_SET_METHOD(module, "exports", RunCallback);
}

NODE_MODULE(delaycb, Init)
