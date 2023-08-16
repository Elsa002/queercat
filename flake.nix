{
  description = "a version of lolcat with some lgbtq+ pride flags options";

  inputs = {
    nixpkgs.url = github:nixos/nixpkgs/nixos-unstable;
    flake-utils.url = github:numtide/flake-utils;
  };

  outputs = {self, nixpkgs, flake-utils, ...}@inputs: flake-utils.lib.eachDefaultSystem (system:
    let 
      pkgs = import nixpkgs {
        inherit system;
      };
      queercat = (with pkgs; stdenv.mkDerivation {
          name = "queercat";
          src = ./.;
          nativeBuildInputs = [ cmake ];
          buildPhase = "make -j $NIX_BUILD_CORES";
          installPhase = ''
            mkdir -p $out/bin
            mv queercat $out/bin
          '';
        }
      );
    in rec {
      defaultApp = flake-utils.lib.mkApp {
        drv = defaultPackage;
      };
      defaultPackage = queercat;
      devShell = pkgs.mkShell {
        buildInputs = [
          queercat
        ];
      };
    }
  );
}