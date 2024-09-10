
zephyr_sources(storage.c)
zephyr_sources(nfc.c)

zephyr_sources_ifdef(CONFIG_BEECHAT_ELERIUM_WALLET wallet.c)
zephyr_sources_ifdef(CONFIG_BEECHAT_ELERIUM_URL_SIGN url_sign.c)
zephyr_sources_ifdef(CONFIG_TINYCRYPT crypto_tinycrypt.c)
zephyr_sources_ifdef(CONFIG_LOG logging.c)

