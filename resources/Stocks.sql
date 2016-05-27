PRAGMA foreign_keys = OFF;

-- ----------------------------
-- Table structure for Categories
-- ----------------------------
DROP TABLE IF EXISTS "main"."Categories";
CREATE TABLE "Categories" (
"ID"  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
"Name"  TEXT NOT NULL,
"SystemCode"  TEXT
);

-- ----------------------------
-- Records of Categories
-- ----------------------------
INSERT INTO "main"."Categories" VALUES (0, '沪深A股', '');
INSERT INTO "main"."Categories" VALUES (1, '自选股', null);

-- ----------------------------
-- Table structure for CategoryMembers
-- ----------------------------
DROP TABLE IF EXISTS "main"."CategoryMembers";
CREATE TABLE "CategoryMembers" (
"ID"  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
"CategoryID"  INTEGER NOT NULL,
"StockCode"  TEXT NOT NULL,
CONSTRAINT "FK_CategoryID" FOREIGN KEY ("CategoryID") REFERENCES "Categories" ("ID") ON DELETE CASCADE ON UPDATE CASCADE,
CONSTRAINT "FK_StockCode" FOREIGN KEY ("StockCode") REFERENCES "Stocks" ("Code") ON DELETE RESTRICT ON UPDATE CASCADE
);

-- ----------------------------
-- Records of CategoryMembers
-- ----------------------------

-- ----------------------------
-- Table structure for DailyTradeinfo
-- ----------------------------
DROP TABLE IF EXISTS "main"."DailyTradeinfo";
CREATE TABLE "DailyTradeinfo" (
"Code"  TEXT NOT NULL,
"TradeDate"  date NOT NULL,
"Open"  REAL NOT NULL DEFAULT 0,
"High"  REAL NOT NULL DEFAULT 0,
"Low"  REAL NOT NULL DEFAULT 0,
"Close"  REAL NOT NULL DEFAULT 0,
"PreClose"  REAL NOT NULL DEFAULT 0,
"Change"  REAL NOT NULL DEFAULT 0,
"ChgPercent"  REAL NOT NULL DEFAULT 0,
"Volume"  REAL NOT NULL DEFAULT 0,
"Turnover"  REAL NOT NULL DEFAULT 0,
"ExchangeRatio"  REAL NOT NULL DEFAULT 0,
"TradableMarketCap"  REAL NOT NULL DEFAULT 0,
"MarketCap"  REAL NOT NULL DEFAULT 0,
"PE"  REAL NOT NULL DEFAULT 0,
"Earnings"  REAL NOT NULL DEFAULT 0,
CONSTRAINT "FK_Code" FOREIGN KEY ("Code") REFERENCES "Stocks" ("Code") ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------
-- Records of DailyTradeinfo
-- ----------------------------


-- ----------------------------
-- Table structure for Stocks
-- ----------------------------
DROP TABLE IF EXISTS "main"."Stocks";
CREATE TABLE "Stocks" (
"Code"  TEXT NOT NULL,
"Name"  TEXT NOT NULL,
"ListingDate"  date,
PRIMARY KEY ("Code")
);

-- ----------------------------
-- Records of Stocks
-- ----------------------------
