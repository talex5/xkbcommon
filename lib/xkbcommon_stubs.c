#include <sys/mman.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/fail.h>

#define Context_val(v) (*((struct xkb_context **) Data_custom_val(v)))

static void finalize_context(value v) {
  xkb_context_unref(Context_val(v));
  Context_val(v) = NULL;
}

static struct custom_operations context_ops = {
  "xkb_context",
  finalize_context,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
  custom_fixed_length_default
};

CAMLprim value caml_xkb_context_new(value v_unit) {
  CAMLparam0();
  CAMLlocal1(v);

  v = caml_alloc_custom_mem(&context_ops, sizeof(struct xkb_context *), 2048);
  Context_val(v) = NULL;

  struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (!ctx) {
    caml_failwith("xkb_context_new: got NULL");
  }
  Context_val(v) = ctx;
    
  CAMLreturn(v);
}

#define Keymap_val(v) (*((struct xkb_keymap **) Data_custom_val(v)))

static void finalize_keymap(value v) {
  xkb_keymap_unref(Keymap_val(v));
  Keymap_val(v) = NULL;
}

static struct custom_operations keymap_ops = {
  "xkb_keymap",
  finalize_keymap,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
  custom_fixed_length_default
};

CAMLprim value caml_xkb_keymap_from_fd(value v_ctx, value v_fd, value v_size) {
  CAMLparam1(v_ctx);
  CAMLlocal1(v);
  struct xkb_context *ctx = Context_val(v_ctx);
  int fd = Int_val(v_fd);
  int size = Int_val(v_size);

  v = caml_alloc_custom_mem(&keymap_ops, sizeof(struct xkb_keymap *), size);
  Keymap_val(v) = NULL;

  char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (map_shm == MAP_FAILED)
    caml_failwith("xkb_context_new: mmap failed");

  struct xkb_keymap *keymap = xkb_keymap_new_from_string(
      ctx, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap(map_shm, size);
  if (!keymap)
    caml_failwith("xkb_keymap_new_from_string returned NULL");
  Keymap_val(v) = keymap;

  CAMLreturn(v);
}

#define State_val(v) (*((struct xkb_state **) Data_custom_val(v)))

static void finalize_state(value v) {
  xkb_state_unref(State_val(v));
  State_val(v) = NULL;
}

static struct custom_operations state_ops = {
  "xkb_state",
  finalize_state,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
  custom_fixed_length_default
};

CAMLprim value caml_xkb_state_new(value v_keymap) {
  CAMLparam1(v_keymap);
  CAMLlocal1(v);
  struct xkb_keymap *keymap = Keymap_val(v_keymap);

  v = caml_alloc_custom_mem(&state_ops, sizeof(struct xkb_state *), 100);
  State_val(v) = NULL;

  struct xkb_state *state = xkb_state_new(keymap);
  if (!state)
    caml_failwith("xkb_state_new returned NULL");
  State_val(v) = state;

  CAMLreturn(v);
}

CAMLprim value caml_xkb_state_key_get_one_sym(value v_state, value v_key) {
  xkb_keysym_t sym = xkb_state_key_get_one_sym(State_val(v_state), Int32_val(v_key));
  return Val_int(sym);
}

CAMLprim value caml_xkb_keysym_get_name(value v_keysym) {
  xkb_keysym_t sym = Int_val(v_keysym);
  char buf[64];
  xkb_keysym_get_name(sym, buf, sizeof(buf));
  /* "If the buffer passed is too small, the string is truncated (though still
      NUL-terminated); a size of at least 64 bytes is recommended." */
  return caml_copy_string(buf);
}

CAMLprim value caml_xkb_state_key_get_utf8(value v_state, value v_key) {
  CAMLparam2(v_state, v_key);
  CAMLlocal1(v);
  struct xkb_state *state = State_val(v_state);
  xkb_keycode_t keycode = Int32_val(v_key);
 
  int size = xkb_state_key_get_utf8(state, keycode, NULL, 0);
  if (size < 1)
    CAMLreturn(caml_copy_string(""));

  v = caml_alloc_string(size);
  xkb_state_key_get_utf8(state, keycode, (char *) String_val(v), size + 1);

  CAMLreturn(v);
}

CAMLprim value caml_xkb_state_update_mask(
    value v_state,
    value v_mods_depressed,
    value v_mods_latched,
    value v_mods_locked,
    value v_group) {
  xkb_state_update_mask(State_val(v_state),
               Int32_val(v_mods_depressed),
	       Int32_val(v_mods_latched),
	       Int32_val(v_mods_locked),
	       0,
	       0,
	       Int32_val(v_group));
  return Val_unit;
}

CAMLprim value caml_xkb_keymap_key_repeats(value v_keymap, value v_key) {
  return Val_bool(xkb_keymap_key_repeats(Keymap_val(v_keymap), Int32_val(v_key)));
}

CAMLprim value caml_xkb_keysym_from_name(value v_name, value v_nocase) {
  return Val_int(xkb_keysym_from_name(String_val(v_name), Bool_val(v_nocase)));
}

CAMLprim value caml_xkb_state_update_key(value v_state, value v_key, value v_down) {
  xkb_state_update_key(State_val(v_state), Int32_val(v_key),
      Bool_val(v_down) ? XKB_KEY_DOWN : XKB_KEY_UP);
  return Val_unit;
}

CAMLprim value caml_xkb_keymap_key_get_name(value v_keymap, value v_key) {
  CAMLparam2(v_keymap, v_key);
  CAMLlocal1(v);
  const char *name = xkb_keymap_key_get_name(Keymap_val(v_keymap), Int32_val(v_key));
  if (name) {
    v = caml_copy_string(name);
    CAMLreturn(caml_alloc_some(v));
  } else {
    CAMLreturn(Val_none);
  }
}

CAMLprim value caml_xkb_keymap_key_by_name(value v_keymap, value v_name) {
  return caml_copy_int32(xkb_keymap_key_by_name(Keymap_val(v_keymap), String_val(v_name)));
}

CAMLprim value caml_xkb_keymap_mod_get_index(value v_keymap, value v_name) {
  xkb_mod_index_t i = xkb_keymap_mod_get_index(Keymap_val(v_keymap), String_val(v_name));
  if (i == XKB_MOD_INVALID)
    return Val_none;
  else
    return caml_alloc_some(Val_int(i));
}

CAMLprim value caml_xkb_state_mod_index_is_active(value v_state, value v_idx) {
  return Val_bool(xkb_state_mod_index_is_active(State_val(v_state), Int_val(v_idx), XKB_STATE_MODS_EFFECTIVE));
}
