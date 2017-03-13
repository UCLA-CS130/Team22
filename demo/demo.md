<title>Team 22</title>

# Team 22 - Webserver Features Demo

Kevin Zhu, Justin Teo, David Stephan

## Features

### Authentication

#### demo

[/private/axolotl.jpg](/private/axolotl.jpg)

Login with user1:password1. Cookie lasts for 10 seconds. Go to /private/seal.gif for alternate image

#### description

Ability to set authentication on certain directories.
Cookies are enabled to keep authentication status and are valid until our specified timeout.

#### config

Same as StaticHandler, but add the following in the config block section

`authentication_list <authentication_txt_file>`
`timeout <cookie_expiration_in_sec>`

#### example

	path /private StaticHandler {
		root private;
		authentication_list authentication.txt;
		timeout 10;
	}

#### details

- username and password login page if accessing restricted directory
- if authenticated, cookie given to browser to keep authentication for certain timeout
	- server also stores authenticated cookies and clears expired cookies before authentication check
- when POST form received, 302 found and redirect sent back so that refreshes do not resend POST data
- currently storing usernames and passwords as plaintext, but could be implemented fully with database later

### Markdown

#### demo

This [current page](demo.md) is in markdown!

#### config

`path <prefix> MarkdownHandler { root <root-dir>; }`

- root-dir: path to serve files from

#### details

- incredibly short (~10 lines)
- markdown library credits to 
- the markdown library is a little shaky (ex: \`\`\` doesn't work)


### Regex matching

#### config

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

## Extras

- [Config for this demo](democonfig)
- [Status page](/status)
- [Sleep handler](/zzz)
- [GitHub](https://github.com/UCLA-CS130/Team22)


![alt text](quokka.jpg "Logo Title Text 1")

^^ proof that static files still work