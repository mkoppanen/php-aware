PHP_ARG_ENABLE(aware-files, whether to enable aware files backend,
[  --enable-aware-files     Enable aware files backend])

if test "$PHP_AWARE_FILES" != "no"; then
  PHP_NEW_EXTENSION(aware_files, aware_files.c, $ext_shared)
  PHP_ADD_EXTENSION_DEP(aware_files, aware)
fi