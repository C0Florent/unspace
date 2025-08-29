{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.05";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells."${system}".default = pkgs.mkShell {
        packages = with pkgs; [
          pkg-config
          valgrind
          gnumake
          libgcc
          (pkgs.writeShellScriptBin "run" ''make "$1" && exec -- env -a "$1" "$(realpath "$1")" "''${@:2}"'')
        ];
      };
    }
  ;
}
