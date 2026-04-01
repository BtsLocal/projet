<?php

    $temp = $_GET['temp'];


    $date = date('Y-m-d');
    $heure = date('H:i:s');


    include ('ouvrirBDD.php');
  

    try
    {

    
       $sql = "INSERT INTO mesures_temperature (temperature, date_mesure, capteur) VALUES ('$temp', NOW(), 'arduino')";
       $bdd->exec($sql);
       echo "Enregistrement fait";
    }   
    catch (Exception $e)
    {
        die('Erreur : ' . $e->getMessage());
        echo "Erreur";
    }

    $bdd = null;
?> 
