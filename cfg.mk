# mini-httpd/cfg.mk

gnu_rel_host    := dl.sv.nongnu.org
upload_dest_dir_:= /releases/mini-httpd/
old_NEWS_hash   := 04744b90f9b8974c338adb28f3d474dc
gpg_key_ID      := 99089D72
url_dir_list    := http://download.savannah.nongnu.org/releases/mini-httpd
today           := $(date "+%Y-%m-%d")
TAR_OPTIONS     += --mtime=$(today)
