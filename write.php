<?php
    // log in vao database
    include("config.php");
    // doc user input
    $rtime = $_POST["t_r"];
    $ytime = $_POST["t_y"];
    $gtime = $_POST["t_g"];
    $station = $_POST["station"];
    // update lai database
    $sql = "update iotled set station=$station,red=$rtime,yellow=$ytime,green=$gtime,send=1" ;
    mysqli_query($conn, $sql);

    mysqli_close($conn);


?>