<style>
html {max-width: 600px; margin-left: 20px;}
h2 {font-size: 30px;}
h3 {font-size: 25px;}
</style>

<title>Team 22</title>



# Team 22 - Webserver Features Demo

Kevin Zhu, Justin Teo, David Stephan

## Features

### Authentication

#### description

Ability to set authentication on certain directories.
Cookies are enabled to keep authentication status and are valid until our specified timeout.

#### demo

[/private/axolotl.jpg](/private/axolotl.jpg)

Login with user1:password1. Cookie lasts for 10 seconds. Go to /private/seal.gif for alternate image

#### config

Same as StaticHandler, but add the following in the config block section

	authentication_list <authentication_txt_file>
	timeout <cookie_expiration_in_sec>

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
- currently storing usernames and passwords as plaintext for proof of concept, but code is laid out such that a database can be easily implemented later

### Markdown

#### description

Converts a markdown text file to HTML style format.

#### demo

This [current page](../definitelystatic/demo.md) is in markdown!

#### config

Same as StaticHandler, except with a different type

`path <prefix> MarkdownHandler { root <root-dir>; }`

#### details
- The API of our StaticHandler allows for easy extension. The MarkdownHandler was trivial to implement (~10 lines).
- Markdown library credits to [Chad Nelson](https://sourceforge.net/projects/cpp-markdown/)
    - The markdown library is a little shaky (\`\`\` doesn't work); however, it should be very easy to replace


### Regex matching

#### description

Allows for a more complex mapping of URIs to request handlers.

#### demo

	# config sampled from the currently running config
	path /demo StaticHandler {
		root demo;
	}
	path_regex /demo \.md MarkdownHandler {
		root demo;
	}

This page was served by the MarkdownHandler while the quokka.jpg below was served by the StaticHandler. Notice how both have the same prefix "/demo".

#### config

`path_regex <prefix> <regex> <handler> {<config>}`

#### details

- regex matches take priority over fixed matches.
- ecmascript syntax
- {, }, " are not supported yet (would require adding escape sequences in the config)

## Extras

- [Full config for this demo](democonfig)
- [Status page](/status) - our status handler is pretty cool
- [Sleep handler](/zzz) - sleeps for 3 seconds
- [GitHub link](https://github.com/UCLA-CS130/Team22)


![alt text](quokka.jpg "Logo Title Text 1")

^^ proof that static files still work
