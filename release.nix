/* Build instructions for the continuous integration system Hydra. */

{ miniHttpdSrc ? { outPath = ./.; revCount = 0; gitTag = "dirty"; }
, officialRelease ? false
}:

let
  pkgs = import <nixpkgs> { };
  version = miniHttpdSrc.gitTag;
in
rec {

  tarball = pkgs.releaseTools.sourceTarball {
    name = "mini-httpd-tarball";
    src = miniHttpdSrc;
    inherit version officialRelease;
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

  build = { system ? "x86_64-linux" }: pkgs.releaseTools.nixBuild {
    name = "mini-httpd-${version}";
    src = tarball;
    buildInputs = with pkgs; [ boostHeaders ];
  };

}
