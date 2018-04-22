<?php
while(true) {
// sleep 10 sec and run again
sleep(20);
exec('php /var/www/tools/get_blocks.php');
}
?>
