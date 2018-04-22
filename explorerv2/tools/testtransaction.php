<?php
     function call_getnewaddress()
	 {
		 $request = array(
					'method' => 'getnewaddress',
					'id' => 1
					);
		$request = json_encode($request);
		$opts = array ('http' => array (
											'method'  => 'POST',
											'header'  => 'Content-type: application/json',
											'content' => $request
											));
		$context  = stream_context_create($opts);
		if ($response = file_get_contents('http://zhao:gang@127.0.0.1:9877', false, $context)) {
			$obj = json_decode($response);
			return $obj;
		} else {
			   throw new Exception('Unable to connect to ');
		}
	 }
	 function call_sendtoaddress($address)
	 {
		 $r = mt_rand(10,100);
		 $request = array(
					'method' => 'sendtoaddress',
					'id' => 2,
					'params' =>array($address,$r)
					);
		$request = json_encode($request);
		//echo $request;
		$opts = array ('http' => array (
											'method'  => 'POST',
											'header'  => 'Content-type: application/json',
											'content' => $request
											));
		$context  = stream_context_create($opts);
		if ($response = file_get_contents('http://zhao:gang@127.0.0.1:9877', false, $context)) {
			$obj = json_decode($response);
			return $obj;
		} else {
			   throw new Exception('Unable to connect to ');
		}
	 }
	
	 $starttime = explode(' ',microtime());
	 echo microtime();

	$i =0;
        while ($i<100000000)
        {
        $i++;
	$obj = call_getnewaddress();
	$newaddress = $obj->{'result'}; 
	//echo $newaddress;
	$obj = call_sendtoaddress($newaddress);
	$res = $obj->{'result'};
	//echo $res;
           $t = mt_rand(1,16);
	sleep($t);
        }

	 $endtime = explode(' ',microtime());
	 $thistime = $endtime[0]+$endtime[1]-($starttime[0]+$starttime[1]);
 	$thistime = round($thistime,3);
 	echo "本网页执行耗时：".$thistime." 秒。".time();

?>






