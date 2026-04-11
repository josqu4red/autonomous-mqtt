let
  nixpkgs-esp-dev = builtins.fetchGit {
    url = "https://github.com/josqu4red/nixpkgs-esp-dev";
  };

  pkgs = import <nixpkgs> {
    overlays = [ (import "${nixpkgs-esp-dev}/overlay.nix") ];

    # The Python library ecdsa is marked as insecure, but we need it for esptool.
    # See https://github.com/mirrexagon/nixpkgs-esp-dev/issues/109
    config.permittedInsecurePackages = [
      "python3.13-ecdsa-0.19.1"
    ];
  };
in
pkgs.mkShell {
  name = "autonomous-mqtt";
  buildInputs = with pkgs; [
    esp-idf-full
    clang-tools
  ];
}
