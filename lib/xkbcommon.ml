module Keycode = struct
  type t = int32
end

module Keysym = struct
  include Keysyms

  (* Keysyms are 32-bit integers with the 3 most significant bits always set to zero. *)
  external raw_get_name : int -> string = "caml_xkb_keysym_get_name"
  external raw_from_name : string -> bool -> int = "caml_xkb_keysym_from_name"

  let from_name ~case_insensitive x = of_int (raw_from_name x case_insensitive)
  let get_name t = raw_get_name (to_int t)
end

module Context = struct
  type t

  external create : unit -> t = "caml_xkb_context_new"
end

module Mod = struct
  type t = private int
end

module Keymap = struct
  type t

  external from_fd : Context.t -> Unix.file_descr -> int -> t = "caml_xkb_keymap_from_fd"
  external key_repeats : t -> Keycode.t -> bool = "caml_xkb_keymap_key_repeats"
  external key_get_name : t -> Keycode.t -> string option = "caml_xkb_keymap_key_get_name"
  external key_by_name : t -> string -> Keycode.t = "caml_xkb_keymap_key_by_name"
  external mod_get_index : t -> string -> Mod.t option = "caml_xkb_keymap_mod_get_index"
end

module State = struct
  type t

  external create : Keymap.t -> t = "caml_xkb_state_new"
  external raw_key_get_one_sym : t -> Keycode.t -> int = "caml_xkb_state_key_get_one_sym"
  external key_get_utf8 : t -> Keycode.t -> string = "caml_xkb_state_key_get_utf8"
  external update_mask : t -> int32 -> int32 -> int32 -> int32 -> unit = "caml_xkb_state_update_mask"
  external update_key : t -> Keycode.t -> bool -> unit = "caml_xkb_state_update_key"
  external mod_is_active : t -> Mod.t -> bool = "caml_xkb_state_mod_index_is_active"

  let key_get_one_sym t k = Keysym.of_int (raw_key_get_one_sym t k)

  let update_mask t ~mods_depressed ~mods_latched ~mods_locked ~group =
    update_mask t mods_depressed mods_latched mods_locked group

  let update_key t k dir =
    update_key t k (match dir with `Up -> false | `Down -> true)
end
