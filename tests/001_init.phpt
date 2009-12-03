--TEST--
Test aware loading
--SKIPIF--
<?php if (!extension_loaded("aware")) print "skip"; ?>
--FILE--
<?php
echo "Aware loaded";
?>
--EXPECT--
Aware loaded
