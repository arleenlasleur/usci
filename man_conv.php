<?php
if(PHP_SAPI !== 'cli') return;

$preamble = "11110100";
$padding  = "00000000";
$payload  = "2";

$msg="";
for($i=0;$i<strlen($payload);$i++){   // text2bin
  $newbyte=decbin(ord($payload[$i]));
  $padlen=8-strlen($newbyte);
  $pad=substr($padding,0,$padlen);
  $msg.=$pad;
  $msg.=$newbyte;
} 
$msg=$preamble.$msg;

$rawmsg="";
for($i=0;$i<strlen($msg);$i++){       // bin2man
  if($msg[$i]=="1") $rawmsg.="01";
   else             $rawmsg.="10";
}

if($rawmsg[0]=="0") $rawmsg=substr($rawmsg,1);  // kill lead+trail zero
if($rawmsg[strlen($rawmsg)-1]=="0") $rawmsg=substr($rawmsg,0,strlen($rawmsg)-1);
$rawmsg=str_replace("11","3",$rawmsg);
$rawmsg=str_replace("00","2",$rawmsg);

$f=fopen("man_code.txt","w");         // man2lst
for($i=0;$i<strlen($rawmsg);$i++) switch($rawmsg[$i]){
  case "0": fputs($f,"file 'man_0s.wav'".PHP_EOL); break;
  case "1": fputs($f,"file 'man_1s.wav'".PHP_EOL); break;
  case "2": fputs($f,"file 'man_0d.wav'".PHP_EOL); break;
  case "3": fputs($f,"file 'man_1d.wav'".PHP_EOL); break;
}
fclose($f);

?>