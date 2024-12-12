<?php
error_reporting(E_ALL);
$dbconn = new mysqli("localhost", "root","", "emcchain32");
// Check connection
if ($dbconn->connect_error) {
    die("Connection failed: " . $dbconn->connect_error);
}

require_once '/var/www/tools/include/jsonRPCClient.php';
$emercoin = new jsonRPCClient('http://zhao:gang@127.0.0.1:6662/');
?>
