/* Build instructions for the continuous integration system Hydra. */

{ miniHttpdSrc ? { outPath = ./.; revCount = 0; gitTag = "dirty"; }
, officialRelease ? false
}:

let
  pkgs = import <nixpkgs> { };
  version = miniHttpdSrc.gitTag;
  versionSuffix = "";
in
rec {

  tarball = pkgs.releaseTools.sourceTarball {
    name = "mini-httpd-tarball";
    src = miniHttpdSrc;
    inherit version versionSuffix officialRelease;
    buildInputs = with pkgs; [ git perl asciidoc libxml2 asciidoc texinfo xmlto docbook2x
                               docbook_xsl docbook_xml_dtd_45 libxslt boostHeaders
                             ];
    postUnpack = ''
      cp -r ${pkgs.gnulib}/ gnulib/
      chmod -R u+w gnulib
      patchShebangs gnulib
      ln -s ../gnulib $sourceRoot/gnulib
    '';
    distPhase = ''
      make distcheck
      mkdir $out/tarballs
      mv -v mini-httpd-*.tar* $out/tarballs/
    '';
  };

  build = pkgs.lib.genAttrs [ "x86_64-linux" "x86_64-freebsd" ] (system:
    with import <nixpkgs> { inherit system; };
    releaseTools.nixBuild {
      name = "mini-httpd";
      src = tarball;
      buildInputs = with (import <nixpkgs> { inherit system; }); [ boostHeaders ];
    });
}
