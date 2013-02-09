# mini-httpd/cfg.mk

gnu_rel_host    := dl.sv.nongnu.org
upload_dest_dir_:= /releases/mini-httpd/
old_NEWS_hash   := f4a21f4b1fe7d5427f247ad0e4e3016b
gpg_key_ID      := 99089D72
url_dir_list    := http://download.savannah.nongnu.org/releases/mini-httpd
today           := $(date "+%Y-%m-%d")
TAR_OPTIONS     += --mtime=$(today)
