# Build a tag system for multiple tables

## Tables
Tags are stored as 5 bytes hex for each symbol, include a leading space. When searching 

### Symbol table
Create a simple table with 2 columns
1 NUM INTEGER
2 VARCHAR(0) VALUE

### Usage table
1   UUID    UUID OF RECORD
2   TABLE   VARCHAR(TableName)
3   VARCHAR(255)    TAGS


