
<?php
    $code=isset($_GET['code']) ? $_GET['code'] : null;
    
    if($code===null){
    	echo "ERREUR: aucun code fourni";
    	exit;
    }

    include ('ouvrirBDD.php');

    try
    {    
       $sql = "SELECT * FROM digicode WHERE code= :code";
       $response=$bdd->prepare($sql);
       $response->execute([':code' => $code]);
       
       if($response -> rowCount() > 0){
       	$row=$response-> fetch();
       	echo "OK";
       } else{
       	echo"REFUSÉE"; 
       }
    }   
    catch (Exception $e)
    {
        echo "Erreur". $e->getMessage();
    }

    $bdd = null;
?> 
