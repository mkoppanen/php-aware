--TEST--
Test set_error_handler overloading error levels
--SKIPIF--
<?php if (!extension_loaded("aware")) print "skip"; ?>
--FILE--
<?php

var_dump(ini_get("aware.enabled"));

function my_error_handler() {
    echo "Called\n";
}

set_error_handler('my_error_handler', E_WARNING);

sdfsfsdf;

foreach (null as $k) {}

?>
--EXPECTF--
string(1) "1"

Notice: Use of undefined constant sdfsfsdf - assumed 'sdfsfsdf' in /tmp/aware/tests/003_error_handler_levels.php on line %d
Called