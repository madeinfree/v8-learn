// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

using namespace v8;

int x, y;

void GetX(Local<String> property, const PropertyCallbackInfo<Value>& info) {
  printf("%s\n", "Get X, OK");
  info.GetReturnValue().Set(x);
}
void SetX(Local<String> property, Local<Value> value,
          const PropertyCallbackInfo<void>& info) {
  printf("%s\n", "Set X, OK");
  x = value->Int32Value();
}
void GetY(Local<String> property, const PropertyCallbackInfo<Value>& info) {
  printf("%s\n", "Get Y, OK");
  info.GetReturnValue().Set(y);
}
void SetY(Local<String> property, Local<Value> value,
          const PropertyCallbackInfo<void>& info) {
  printf("%s\n", "Set Y, OK");
  y = value->Int32Value();
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
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    // Create a new context.
    // v8::Local<v8::Context> context = v8::Context::New(isolate);

    // strncpy(sname, "OK\n", sizeof(sname));

    Local<ObjectTemplate> global_template = ObjectTemplate::New(isolate);
    // global_template->SetInternalFieldCount(1);
    global_template->SetAccessor(String::NewFromUtf8(isolate, "x"), GetX, SetX);
    global_template->SetAccessor(String::NewFromUtf8(isolate, "y"), GetY, SetY);
    Local<Context> context = Context::New(isolate, NULL, global_template);
    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    // Create a string containing the JavaScript source code.
    v8::Local<v8::String> source =
        v8::String::NewFromUtf8(isolate, "x=10; x; y=5; y;",
                                v8::NewStringType::kNormal)
            .ToLocalChecked();

    // Compile the source code.
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();

    // Run the script to get the result.
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    String::Utf8Value utf8(isolate, result);
    // printf("%s\n", *utf8);
    printf("result x => %d\nresult y => %d\n", x, y);
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::ShutdownPlatform();
  delete platform;
  delete create_params.array_buffer_allocator;
  return 0;
}
