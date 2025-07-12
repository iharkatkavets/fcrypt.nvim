#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"
#include "decrypt.h"
#include "encrypt.h"
#include "random.h"

int lua_get_fcrypt_version(lua_State *L) {
  lua_pushstring(L, FCRYPT_VERSION);
  return 1;
}

static int lua_fcrypt_decrypt_buf(lua_State *L) {
  size_t in_len, key_len;
  const char *in_str = luaL_checklstring(L, 1, &in_len);
  const char *key_str = luaL_checklstring(L, 2, &key_len);
  size_t required_len = 0;

  int result = fcrypt_decrypt_buf(
    (const uint8_t *)in_str, in_len,
    (uint8_t *)key_str, key_len,
    NULL, 0, 
    &required_len);

  if (result != EXIT_SUCCESS) {
    return luaL_error(L, "decryption failed");
  }

  uint8_t *decrypted_buf = malloc(required_len);
  size_t decrypted_buf_len = 0;
  result = fcrypt_decrypt_buf(
    (const uint8_t *)in_str, in_len,
    (uint8_t *)key_str, key_len,
    decrypted_buf, required_len, 
    &decrypted_buf_len);

  if (result != EXIT_SUCCESS) {
    return luaL_error(L, "decryption failed");
  }

  lua_pushlstring(L, (const char *)decrypted_buf, decrypted_buf_len);
  free(decrypted_buf);
  return 1;
}

static int lua_fcrypt_encrypt_buf(lua_State *L) {
  size_t in_len, key_len, hint_len;
  const char *in_str = luaL_checklstring(L, 1, &in_len);
  const char *key_str = luaL_checklstring(L, 2, &key_len);
  const char *hint_str = luaL_checklstring(L, 3, &hint_len);
  uint16_t pad_size = 0; 

  if (fcrypt_gen_uint16(&pad_size) != EXIT_SUCCESS) {
    return luaL_error(L, "failed to generate pad_size");
  }

  size_t required_len;
  int result = fcrypt_encrypt_buf(
    (const uint8_t *)in_str, in_len,
    (const uint8_t *)key_str, key_len,
    (const uint8_t *)hint_str, hint_len, 
    pad_size,
    NULL, 0, 
    &required_len);
  if (result != EXIT_SUCCESS) {
    return luaL_error(L, "encryption failed");
  }

  size_t out_buf_size = 0;
  uint8_t *out_buf = malloc(required_len);
  if (!out_buf) {
    return luaL_error(L, "failed to allocate buf");
  }

  result = fcrypt_encrypt_buf(
    (const uint8_t *)in_str, in_len,
    (const uint8_t *)key_str, key_len,
    (const uint8_t *)hint_str, hint_len, 
    pad_size,
    out_buf, required_len, 
    &out_buf_size);
  if (result != EXIT_SUCCESS) {
    free(out_buf);
    return luaL_error(L, "encryption failed");
  }

  lua_pushlstring(L, (const char *)out_buf, out_buf_size);
  free(out_buf);

  return 1;
}

int luaopen_fcrypt_lua_mod(lua_State *L) {
  luaL_Reg funcs[] = {
    {"get_fcrypt_version", lua_get_fcrypt_version},
    {"decrypt_buf", lua_fcrypt_decrypt_buf},
    {"encrypt_buf", lua_fcrypt_encrypt_buf},
    {NULL, NULL}
  };

  luaL_newlib(L, funcs);
  return 1;
}
