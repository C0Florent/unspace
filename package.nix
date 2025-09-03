{ stdenv, lib }:

stdenv.mkDerivation (finalAttrs: {
  src = ./.;
  pname = "unspace";
  version = "0.1.0";

  meta = {
    description = "Simple CLI to remove spaces from filenames";
    homepage = "https://github.com/C0Florent/unspace";
    license = lib.licenses.mit;
    mainProgram = "unspace";
    platforms = lib.platforms.linux;
  };

  installPhase = ''
    runHook preInstall

    mkdir -p "$out/bin"
    mv ${finalAttrs.pname} "$out/bin"

    runHook postInstall
  '';
})
