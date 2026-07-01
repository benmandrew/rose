{
  description = "rose";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = [
            pkgs.cmake
            pkgs.clang
            pkgs.clang-tools
            pkgs.cppcheck
            pkgs.cpplint
            pkgs.git
            pkgs.nodejs_18
            pkgs.fmt
          ] ++ pkgs.lib.optionals pkgs.stdenv.isLinux [
            pkgs.gcc
          ];
        };
      });
}
