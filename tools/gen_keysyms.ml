let () =
  let keys = ref [] in
  for i = 0 to 0x1fffffff do
    if i < 0x01000100 || i > 0x0110ffff then ( (* Exclude generic unicode IDs *)
      if i mod 0x01000000 = 0 then Printf.eprintf "%.0f%%\n%!" (100. *. float i /. 0x1fffffff.);
      let name = Xkbcommon.Keysym.raw_get_name i in
      if not (String.starts_with ~prefix:"0x" name) then (
        Printf.eprintf "%s\n" name;
        keys := (i, name) :: !keys
      )
    )
  done;
  let keys = List.rev !keys in
  Printf.eprintf "Found %d names\n%!" (List.length keys);
  print_endline "(* Auto-generated by gen_keysyms.ml *)";
  print_endline "type t =";
  keys |> List.iter (fun (_i, name) -> Printf.printf "  | K_%s\n" name);
  print_endline "  | Other of int";
  print_endline "";
  print_endline "let of_int = function";
  keys |> List.iter (fun (i, name) -> Printf.printf "  | %d -> K_%s\n" i name);
  print_endline "  | x -> Other x";
  print_endline "";
  print_endline "let to_int = function";
  keys |> List.iter (fun (i, name) -> Printf.printf "  | K_%s -> %d\n" name i);
  print_endline "  | Other x -> x";
