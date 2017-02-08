# php-wkhtmltox
html转pdf或者image的扩展（基于php7）


PHP bindings for the fabulous libwkhtmltox

Special thanks to antialize for creating wkhtmltopdf

For parameters and settings (for users familiar with the command line utility) see:

http://wkhtmltopdf.org/libwkhtmltox/pagesettings.html
Installation

$ phpize
$ ./configure
$ make install
edit your php.ini and add:

extension=phpwkhtmltox.so
Tested on:

CentOS release 6.5 (Final)
PHP 7.0.5 

Pre-requisites on all platforms:

you need libwkhtmltox.* somewhere in your LD path (/usr/local/lib)
you need the directory src/include/wkhtmltox from wkhtmltopdf somewhere on your include path (/usr/local/include)
OSX Notes:

Until this bug is fixed you need qt_menu.nib directory from the QT source tree in the same directory as your libwkhtmltox.* library files
