<title>Team 22</title>

# Team 22 - Webserver Features Demo

Kevin Zhu, Justin Teo, David Stephan

## Features

### 1. Markdown

#### config

`path <prefix> MarkdownHandler { root <root-dir>; }`

- root-dir: path to serve files from


#### details

- incredibly short (~10 lines)
- markdown library credits to 
- the markdown library is a little shaky (ex: \`\`\` doesn't work)


### 2. Regex matching

#### syntax

`path_regex <prefix> <regex> <handler> {<config>}`

#### example


	path /demo StaticHandler {
		root demo;
	}
	path_regex /demo \.md MarkdownHandler {
		root demo;
	}

#### details

- regex matches take priority over fixed matches
- ecmascript syntax
- {, }, " are not supported yet


### 3. Authentication


## Extras

- [Config for this demo](democonfig)
- [Status page](/status)
- [Sleep handler](/zzz)
- [GitHub](https://github.com/UCLA-CS130/Team22)


![alt text](quokka.jpg "Logo Title Text 1")

^^ proof that static files still work