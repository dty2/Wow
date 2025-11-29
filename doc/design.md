# Design

## WorkDir

```
--work-dir set work dir as $WORK_DIR, default is at $XDG_RUNTIME_DIR
local socket listen on $WORK_DIR/wow/wow.sock
```

## IPC

### Request

- refresh: `{ "action": "refresh" }`
- next: `{ "action": "next" }`
- prev: `{ "action": "prev" }`
- manual: `{ "action": "manual" }`
- test:

```
{
  "action": "test",
  "target": "[wallpaper-name]"
}
```

- list:

```
{
  "action": "list",
  "target": "[list-name]"
}
```

- quit: `{ "action": "quit" }`
- info: `{ "action": "info" }`

### Response

`{ "result": "[result-info]" }`
