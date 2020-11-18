# PHP Extension for TON SDK Wrapper

**Community links:**

[![Chat on Telegram](https://img.shields.io/badge/chat-on%20telegram-9cf.svg)](https://t.me/RADIANCE_TON_SDK)

## Supported platforms

 - Windows x86, x64
 - Linux
 - macOS
 
## Installation

See [Installation notes](install.md).

## Development

See [Development notes](development.md).

## Functions

The following functions are added by this extension:

```php
string ton_create_context( string $config_json );
```

Creates new TON client context.

Parameters:

 - `$config_json` - TON client configuration.
 
Return value:

 JSON containing context ID.

```php
void ton_destroy_context( int $context );
```

Destroys TON client context.

Parameters:

 - `$context` - Context ID previously returned by `ton_create_context`.

```php
string ton_request_sync( int $context, string $function_name, string $params_json);
```

Runs TON SDK request synchronously (using `tc_request_sync`).

Parameters:

 - `$context` - Context ID previously returned by `ton_create_context`.
 - `$function_name` - name of the TON SDK function to call.
 - `$params_json` - JSON-encoded function params.
 
Return value:

 JSON response.

```php
resource ton_request_start( int $context, string $function_name, string $params_json );
```

Runs TON SDK request asynchronously using `tc_request_ptr`.

Parameters:

 - `$context` - Context ID previously returned by `ton_create_context`.
 - `$function_name` - name of the TON SDK function to call.
 - `$params_json` - JSON-encoded function params.
 
Return value:

 Request handle.

```php
array ton_request_next( resource $request, [ int $timeout ] );
```

Fetches the next async event.

Parameters:

 - `$request` - Request handle previously returned by `ton_request_start`.
 - `$timeout` - Timeout in milliseconds (optional).
 
Return value:

 Array containing 3 values:
  
```php
 [ string $json, int $status, bool $finished ]
```

 `$json` is always containing callback data unless it's a return value for a function which returns nothing 
 (like `net.unsubscribe_collection`).
  
 `$status` corresponds to the `tc_response_types` enum defined in [tonclient.h](https://github.com/tonlabs/TON-SDK/blob/master/ton_client/client/tonclient.h);
 
 When request is finished `$finished` will be `true`. 

```php
?bool is_ton_request_finished( resource $request )
```

Checks whether the TON SDK request is finished.

Parameters:

 - `$request` - Request handle previously returned by `ton_request_start`.
 
Return value:

 `true` if request has been finished, `false` if not, and `null` if invalid `$request`
 handle is passed to the function arguments.


## Implementation notes

This extension uses threads and blocking queues to work with TON SDK functions and callbacks.
`ton_request_next` is the only blocking call here, all other functions are instant.

Extension is supposed to work in both Thread-Safe and Non-Thread safe environments. 

## License

Apache License, Version 2.0.

## Troubleshooting

Fire any question to our [Telegram channel](https://t.me/RADIANCE_TON_SDK).