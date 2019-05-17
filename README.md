Webserver handling basic HTTP GET requests.

### Usage:

```bash
$ ./server <port> <directory>
```
e.g.
```bash
$ ./server 8888 webpages
```
Note: If we want to access a site with path `webpages/name_of_website/index.html`, there should be an entry in `/etc/hosts` file of form 
```
...
127.0.0.1	name_of_website
...
```
so that we can access it with `http://name_of_website:8888/index.html`

