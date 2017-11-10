// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <string.h>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

using namespace v8;
using namespace std;

int x, y;

void GetX(Local<String> property, const PropertyCallbackInfo<Value>& info) {
  info.GetReturnValue().Set(x);
}
void SetX(Local<String> property, Local<Value> value,
          const PropertyCallbackInfo<void>& info) {
  x = value->Int32Value();
}
void GetY(Local<String> property, const PropertyCallbackInfo<Value>& info) {
  info.GetReturnValue().Set(y);
}
void SetY(Local<String> property, Local<Value> value,
          const PropertyCallbackInfo<void>& info) {
  y = value->Int32Value();
}
void LogCallback(const FunctionCallbackInfo<Value>& args) {
  Local<Value> arg = args[0];
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  String::Utf8Value value(isolate, arg);
  printf("%s\n", *value);
}
MaybeLocal<String> ReadFile(Isolate* isolate) {
  MaybeLocal<String> result;
  char fileName[8] = "main.js";

  FILE* file = fopen("main.js", "r");
  if (file == NULL) {
    printf("檔案開啟失敗");
    return MaybeLocal<String>();
  }

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&chars[i], 1, size - 1, file);
  }
  fclose(file);

  result = String::NewFromUtf8(isolate, chars);

  delete[] chars;
  return result;
}

int main(int argc, char* argv[]) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  v8::Platform* platform = v8::platform::CreateDefaultPlatform();
  v8::V8::InitializePlatform(platform);
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate* isolate = Isolate::New(create_params);
  {
    // Create a stack-allocated handle scope.
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);

    Local<ObjectTemplate> global_template = ObjectTemplate::New(isolate);
    global_template->SetInternalFieldCount(1);
    global_template->SetAccessor(String::NewFromUtf8(isolate, "x"), GetX, SetX);
    global_template->SetAccessor(String::NewFromUtf8(isolate, "y"), GetY, SetY);
    global_template->Set(String::NewFromUtf8(isolate, "log"),
                         FunctionTemplate::New(isolate, LogCallback));
    // Create a new context.
    Local<Context> context = Context::New(isolate, NULL, global_template);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    // Create a string containing the JavaScript source code.
    Local<String> source;
    if (!ReadFile(isolate).ToLocal(&source)) {
      fprintf(stderr, "Error reading \n");
      return 1;
    }

    // Compile the source code.
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();

    // Run the script to get the result.
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    String::Utf8Value utf8(isolate, result);
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  delete create_params.array_buffer_allocator;
  return 0;
}
