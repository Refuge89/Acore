-- MySQL dump 10.13  Distrib 5.6.9-rc, for Win64 (x86_64)
--
-- Host: localhost    Database: unused
-- ------------------------------------------------------
-- Server version	5.6.9-rc

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `cfg_configs`
--

DROP TABLE IF EXISTS `cfg_configs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `cfg_configs` (
  `Id` int(10) NOT NULL DEFAULT '0',
  `RealmType` int(10) NOT NULL DEFAULT '0',
  `PlayerKillingAllowed` int(10) NOT NULL DEFAULT '0',
  `Roleplaying` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `cfg_configs`
--

LOCK TABLES `cfg_configs` WRITE;
/*!40000 ALTER TABLE `cfg_configs` DISABLE KEYS */;
/*!40000 ALTER TABLE `cfg_configs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Records of `cfg_configs`
--

INSERT INTO `cfg_configs` VALUES ('1', '0', '0', '0');
INSERT INTO `cfg_configs` VALUES ('2', '1', '1', '0');
INSERT INTO `cfg_configs` VALUES ('3', '2', '0', '0');
INSERT INTO `cfg_configs` VALUES ('4', '3', '1', '0');
INSERT INTO `cfg_configs` VALUES ('5', '4', '0', '0');
INSERT INTO `cfg_configs` VALUES ('6', '5', '1', '0');
INSERT INTO `cfg_configs` VALUES ('7', '6', '0', '1');
INSERT INTO `cfg_configs` VALUES ('8', '7', '0', '1');
INSERT INTO `cfg_configs` VALUES ('9', '8', '1', '1');
INSERT INTO `cfg_configs` VALUES ('10', '9', '0', '0');
INSERT INTO `cfg_configs` VALUES ('11', '10', '1', '0');
INSERT INTO `cfg_configs` VALUES ('12', '11', '0', '0');
INSERT INTO `cfg_configs` VALUES ('13', '12', '1', '0');
