-- MySQL dump 10.15  Distrib 10.0.28-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: localhost
-- ------------------------------------------------------
-- Server version	10.0.28-MariaDB-2

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `intersection`
--

DROP TABLE IF EXISTS `intersection`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `intersection` (
  `video_id` int(10) NOT NULL,
  `user_id` int(10) NOT NULL,
  `qer_id` int(10) NOT NULL,
  `timestamp` float NOT NULL,
  `intersection` float NOT NULL,
  PRIMARY KEY (`video_id`,`user_id`,`qer_id`,`timestamp`)
) ;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `intersection`
--

--
-- Table structure for table `qer`
--

DROP TABLE IF EXISTS `qer`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `qer` (
  `qer_id` int(10) NOT NULL AUTOINCREMENT,
  `theta` float NOT NULL,
  `phi` float NOT NULL,
  `hdim` float NOT NULL,
  `vdim` float NOT NULL,
  PRIMARY KEY (`qer_id`)
) ;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `qer`
--

--
-- Table structure for table `segments`
--

DROP TABLE IF EXISTS `segments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `segments` (
  `video_id` int(10) NOT NULL,
  `user_id` int(10) NOT NULL,
  `qer_id` int(10) NOT NULL,
  `segment_id` int(10) NOT NULL,
  `segment_duration` float NOT NULL,
  `intersection` float NOT NULL,
  PRIMARY KEY (`video_id`,`user_id`,`qer_id`,`segment_id`,`segment_duration`)
) ;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `segments`
--

-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `user_id` int(10) NOT NULL AUTOINCREMENT,
  `user_name` varchar(20) NOT NULL,
  PRIMARY KEY (`user_id`)
) ;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

--
-- Table structure for table `videos`
--

DROP TABLE IF EXISTS `videos`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `videos` (
  `video_id` int(10) NOT NULL AUTOINCREMENT,
  `video_name` varchar(20) NOT NULL,
  PRIMARY KEY (`video_id`)
) ;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `videos`
--


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-11-28 17:58:27
