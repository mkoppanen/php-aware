--TEST--
Test set_error_handler overloading error levels
--SKIPIF--
<?php if (!extension_loaded("aware")) print "skip"; ?>
--FILE--
<?php

function my_error_handler() {
    echo "Called\n";
}

set_error_handler('my_error_handler', E_WARNING);

sdfsfsdf;

foreach (null as $k) {}

?>
--EXPECT--
Notice: Use of undefined constant sdfsfsdf - assumed 'sdfsfsdf' in /tmp/aware/tests/003_error_handler_levels.php on line 9
Called
