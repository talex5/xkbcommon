module C = Configurator.V1

let get_flags p ~package ~expr =
  match C.Pkg_config.query_expr_err p ~package ~expr with
  | Error e -> failwith (Printf.sprintf "%s(%s): %s" package expr e)
  | Ok x ->
    C.Flags.write_sexp (package ^ "-cflags.sexp") x.cflags;
    C.Flags.write_sexp (package ^ "-clibs.sexp") x.libs

let () =
  C.main ~name:"foo" (fun c ->
    match C.Pkg_config.get c with
      | None -> failwith "pkg-config not found"
      | Some pc -> get_flags pc ~package:"xkbcommon" ~expr:"xkbcommon"
    );
