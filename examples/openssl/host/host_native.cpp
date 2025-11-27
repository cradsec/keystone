//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include <edge_call.h>
#include <keystone.h>

unsigned long
print_string(char* str);
void
print_string_wrapper(void* buffer);
#define OCALL_PRINT_STRING 1

/***
 * An example call that will be exposed to the enclave application as
 * an "ocall". This is performed by an edge_wrapper function (below,
 * print_string_wrapper) and by registering that wrapper with the
 * enclave object (below, main).
 ***/
unsigned long
print_string(char* str) {
  return printf("Enclave said: \"%s\"\n", str);
}

int
main(int argc, char** argv) {
  Keystone::Enclave enclave;
  Keystone::Params params;
  char *runner  = "./openssl-runner";
  const char *cmd, *runtime,*loader;
  char buf_cmd[4096], buf_runtime[4096], buf_loader[4096];
  char *name_cmd="openssl.eapp", *name_runtime="eyrie-rt", *name_loader="loader.bin";

  char *buffer;
  size_t buffer_len, offset;
  int i;

  const char *eapp_path = getenv("KS_EAPP_PATH");
  const char *e = getenv("KS_BIN_DIR");
  if (e != NULL) {
	  snprintf(buf_cmd, sizeof(buf_cmd),"%s/%s", e, name_cmd);
	  snprintf(buf_runtime, sizeof(buf_cmd),"%s/%s", e, name_runtime);
	  snprintf(buf_loader, sizeof(buf_cmd),"%s/%s", e, name_loader);
	  cmd     = buf_cmd;
	  runtime = buf_runtime;
	  loader  = buf_loader;
  } else {
	  cmd     = name_cmd;
	  runtime = name_runtime;
	  loader  = name_loader;
  }
  if (eapp_path != NULL) {
	  cmd = eapp_path;
  }
  printf("cmd %s, runtime %s, loader %s\n", cmd, runtime, loader);
  if (argc > 1) printf("argv[1] %s\n", argv[1]);

  params.setFreeMemSize(24 * 1024 * 1024);
  params.setUntrustedSize(1024 * 1024);

  enclave.init(cmd, runtime, loader, params);

  enclave.registerOcallDispatch(incoming_call_dispatch);

  /* We must specifically register functions we want to export to the
     enclave. */
  register_call(OCALL_PRINT_STRING, print_string_wrapper);

  edge_call_init_internals(
      (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());

  buffer     = (char*)enclave.getSharedBuffer();
  buffer_len = enclave.getSharedBufferSize();
  *((int*)buffer) = argc - 1;

  for (i=1, offset=8; i<argc; i++) {
	if (offset + strlen(argv[i]) + 1 >= buffer_len) {
		printf("buffer too small\n");
		return 1;
	}
	strcpy(buffer + offset, argv[i]);
	offset += strlen(argv[i]) + 1;
  }
  *((int*)(buffer + 4)) = (int)offset - 8;
  uintptr_t retval;
  enclave.run(&retval);

  return retval;
}

/***
 * Example edge-wrapper function. These are currently hand-written
 * wrappers, but will have autogeneration tools in the future.
 ***/
void
print_string_wrapper(void* buffer) {
  /* Parse and validate the incoming call data */
  struct edge_call* edge_call = (struct edge_call*)buffer;
  uintptr_t call_args;
  unsigned long ret_val;
  size_t arg_len;
  if (edge_call_args_ptr(edge_call, &call_args, &arg_len) != 0) {
    edge_call->return_data.call_status = CALL_STATUS_BAD_OFFSET;
    return;
  }

  /* Pass the arguments from the eapp to the exported ocall function */
  ret_val = print_string((char*)call_args);

  /* Setup return data from the ocall function */
  uintptr_t data_section = edge_call_data_ptr();
  memcpy((void*)data_section, &ret_val, sizeof(unsigned long));
  if (edge_call_setup_ret(
          edge_call, (void*)data_section, sizeof(unsigned long))) {
    edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
  } else {
    edge_call->return_data.call_status = CALL_STATUS_OK;
  }

  /* This will now eventually return control to the enclave */
  return;
}
