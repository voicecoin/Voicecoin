<?php
error_reporting(0);
$dbconn = new mysqli("localhost", "dog","zhaogang", "emcchain");
// Check connection
if ($dbconn->connect_error) {
    die("Connection failed: " . $dbconn->connect_error);
}

$dbconn2 = new mysqli("localhost", "dog","zhaogang", "emcchain");
// Check connection
if ($dbconn2->connect_error) {
    die("Connection failed: " . $dbconn2->connect_error);
}

require_once '/var/www/tools/include/jsonRPCClient.php';
$emercoin = new jsonRPCClient('http://zhao:gang@127.0.0.1:9877/');
?>
