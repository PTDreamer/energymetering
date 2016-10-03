<?php
//http://pi1.lan/get_data.php?type=VOLTAGE&start_date=16-09-26&end_date=16-09-27&points=10
//http://pi1.lan/get_data.php?type=VOLTAGE&last=true
 $con = mysqli_connect('127.0.0.1','nodered','XXX','iot');
 if($_GET['last']=='true') {
  	$query = "SELECT value, type FROM energy_measures WHERE type='" . $_GET['type'] . "' ORDER BY id DESC LIMIT 1;";
  	$exec = mysqli_query($con,$query);
 	while($row = mysqli_fetch_array($exec)){
 			echo "{\"value\":".$row['value'].'}';
 	}
 }
 else if($_GET['type']=='dailyENERGY' | $_GET['type']=='monthlyENERGY') {
  	if($_GET['type']=='dailyENERGY') {
  	$query = "SELECT date(`time`) as day, max(value) as max,min(value) as min,max(value) - min(value) as diff
    from `energy_measures`
    where date(`time`) >= '" . $_GET["start_date"] . "' and date(`time`) <= '" . $_GET["end_date"] . "'
       and type='ENERGY' and value<>'-1'
    group by date(`time`);";
	}
  else if($_GET['type']=='monthlyENERGY') {
    $query = "SELECT monthname(`time`) as month, max(value) as max,min(value) as min,max(value) - min(value) as diff
    from `energy_measures`
    where month(`time`) >= month('" . $_GET["start_date"] . "') and month(`time`) <= month('" . $_GET["end_date"] . "') and year(`time`) >= year('" . $_GET["start_date"] . "') and year(`time`) <= year('" . $_GET["end_date"] . "') and value<>'-1'
       and type='ENERGY'
    group by month(`time`), year(`time`);";
  }
	$sth = mysqli_query($con,$query);
	$rows = array();
	while($r = mysqli_fetch_assoc($sth)) {
    	$rows[] = $r;
	}
	print json_encode($rows);
 }
 else {
 	$query = "SELECT COUNT(*) as count FROM energy_measures WHERE `time` >= '" . $_GET["start_date"] . "' AND `time` <= '" . $_GET["end_date"] . "' AND type='".$_GET["type"]."'".";";
 	$exec = mysqli_query($con,$query);
 	$count = mysqli_fetch_array($exec)[0];
 	$number_of_requested_rows = $_GET["points"];
 	$groups_of = floor($count / $number_of_requested_rows);
 	$query = "SELECT MAX(`time`) as time , AVG(`value`) as value FROM (SELECT @rownum:=@rownum+1 `rownum`,  FLOOR(@rownum/".$groups_of.") as 'datagrp', p.* from (SELECT * FROM energy_measures WHERE time >= '" . $_GET["start_date"] . "' AND time <= '" . $_GET["end_date"] . "' AND type='".$_GET["type"]."'".") p, (SELECT @rownum:=0) r ORDER BY `time` ASC) AS T GROUP BY `datagrp`;";

 	$sth = mysqli_query($con,$query);
	$rows = array();
	while($r = mysqli_fetch_assoc($sth)) {
    	$rows[] = $r;
	}
	print json_encode($rows);
}
// Free result set
mysqli_free_result($exec);

mysqli_close($con);
?>
