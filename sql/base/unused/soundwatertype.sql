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
-- Table structure for table `soundwatertype`
--

DROP TABLE IF EXISTS `soundwatertype`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `soundwatertype` (
  `Id` int(10) NOT NULL DEFAULT '0',
  `LiquidType` int(10) NOT NULL DEFAULT '0',
  `FluidSpeed` int(10) NOT NULL DEFAULT '0',
  `Sound` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`),
  UNIQUE KEY `Id` (`Id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `soundwatertype`
--

LOCK TABLES `soundwatertype` WRITE;
/*!40000 ALTER TABLE `soundwatertype` DISABLE KEYS */;
/*!40000 ALTER TABLE `soundwatertype` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Records of `soundwatertype`
--

INSERT INTO `soundwatertype` VALUES ('1', '0', '0', '1111');
INSERT INTO `soundwatertype` VALUES ('2', '0', '4', '1112');
INSERT INTO `soundwatertype` VALUES ('3', '0', '8', '1113');
INSERT INTO `soundwatertype` VALUES ('4', '1', '0', '1114');
INSERT INTO `soundwatertype` VALUES ('5', '1', '4', '1114');
INSERT INTO `soundwatertype` VALUES ('6', '1', '8', '1114');
INSERT INTO `soundwatertype` VALUES ('7', '3', '0', '3880');
INSERT INTO `soundwatertype` VALUES ('8', '3', '4', '3880');
INSERT INTO `soundwatertype` VALUES ('9', '3', '8', '3880');
INSERT INTO `soundwatertype` VALUES ('27', '2', '0', '3072');
INSERT INTO `soundwatertype` VALUES ('28', '2', '4', '3052');
INSERT INTO `soundwatertype` VALUES ('29', '2', '8', '3052');
