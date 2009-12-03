--TEST--
Test aware loading
--SKIPIF--
<?php if (!extension_loaded("aware")) print "skip"; ?>
--FILE--
<?php
echo "Aware loaded\n";
var_dump(ini_get("aware.enabled"));
?>
--EXPECT--
Aware loaded
string(1) "1"