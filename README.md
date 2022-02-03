# WebReader

This project is inspired by the BookBrowser (https://github.com/dmitrj-pro/BookBrowser) project (not original, but modified by me). At the moment, all the main functions have been transferred to this project.

## Features
- Multiple book formats
    - epub
    - pdf
    - galery (Comics pics)
- List view
- Responsive web interface
- Update notifications
- Browse by:
    - Author
    - Base folder name

- Web based reader
    - Custom fonts, colors, sizing, spacing
    - Remembers your position
    - Book search
    - And more
- Save read position
- And more
- Easy-to-use
- No extra dependencies

## Create galery

Create folder in books directory and copy image files to it. Example folder ~/Books/Comics/Konosuba 1.

Create file next to the created folder the same name as the created directory with type ".galery". Example ~/Books/Comics/Konosuba 1.galery.

Optional: Write information about galery in xml format:

```
<galery>
	<name>KonoSuba Part 1</name>
	<cover>Image 1.jpg</cover>
	<author>Natsume Akatsuki</author>
</galery>
```

If option cover is empty, then selected first image from galery for book cover.

## About save position

Current version of WebReader save read position. Position is global for all user. 

In time of read of book WebReader save file with current read position on hard drive.

## Usage

```
-h  {String}	Set listen host
--h | -host | --host | 

-p  {unsigned short}	Set listen port
--p | -port | --port | 

-b  {String}	Set basepath for books
--b | -basepath | --basepath | 

-l  {String}	Set log level application
--l | -log | --log | 

-position  {String}	Set path to position file
--position | 

help 	Write this text
-help | --help | ? | 
```



