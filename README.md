# Numero URI

This project takes a set of given URI templates and encodes them into a number representation. It can also take the output and decode it based on the same given URI templates.

Important to note, the templates file will start empty. The configuration.json and template.json files can be found in the same directory as the executable.

---

## Required Technologies
- [GCC](https://gcc.gnu.org/) - Required
- [CMake](https://cmake.org/) - Required
- [Docker](https://docker-docs.netlify.app/install/) - Optional
- Make [[Windows download](https://gnuwin32.sourceforge.net/packages/make.htm)] - Optional
- [libcurl](https://curl.se/libcurl/c/) - Required for CLI
---
## Target Overview
- `all`         Runs the docker target.
- `docker`      Runs a docker container that build and runs the executable. You can run tests with this target by appending cmd=TEST. Arguments can be passed to the executable by using args='encode ...'.
- `build`       Builds the executable using CMake on your local machine.
- `test`        Runs the tests.
- `clean`       Removes the build directory.

---
## Build Instructions
To build the project executable and library
1. `cmake -B build -DBUILD_TESTS=OFF`
2. `cmake --build build`

To run the executable from the CLI
- build/bin/numero_uri [args]
- Available arguments and formats
    - encode            Encodes a URL depending on the current loaded templates
        - ex. encode https://webex.com/meeting1234/user3213
    - decode            Decodes a uint128 string from the encode function
    - config            Changes a configuration setting (located in executable directory)
        - ex. config template-file /path/to/template.json
    - add-template      Adds a template to the templates file
        - ex. add-template https://www.webex.com\<int24=123\>/meeting\<int16\>/user\<int16\>
    - remove-template   Removes a template from the templates file
        - ex. remove-template 123

To build the tests and run them
1. `cmake -B build -DBUILD_TESTS=ON`
2. `cmake --build build`
3. `ctest --test-dir build/tests --output-on-failure`

--
## Examples
### Docker
- Add a template

    ```make args='add-template https://webex.com<pen=123>/meeting<int16>/room<int16>'```

    Creates a template that receives 3 numbers groups in a URI according to the pattern given. The first group **<pen=123>** is the only time a group is required. The value must be a unique identifier for this template.

- Add a template with a sub-pen

    ```make args='add-template https://webex.com<pen=1234><sub_pen=2>/obj<int16>/ver<int16>'```

    Creates a template that receives 3 numbers groups in a URI according to the pattern given. The second group **<sub_pen=2>** is a sub-template number The value must be a unique identifier for this template.

    Note: It is important to have the sub_pen immediately follow the pen, everything between <pen=xxx> and <sub_pen=xxx> is ignored, similarly, so is everything between <sub_pen> and the next slash (/).

- Encode a template

    ```make args='encode https://webex.com/meeting2/room56'```

    This will output the encoded number string **`0x00000500020038000000000000000000`**. Which can be fed back in using the next command.

- Decode a number string

    ```make args='decode 0x00000500020038000000000000000000'```

    This will output the URI **`https://webex.com/meeting2/room56`** according to the template provided with the unique identifier.

- Remove a template

    ```make args='remove-template 123'```

    Arguments must be the unique identifier for the templates provided.

- Select a template file

    ```make args='config template-file C:/my_files/path/to/template/templates.json'```

    This argument will update the configuration file to have a new path to a template file.

### Local - Linux
- Add a template

    ```./build/bin/numero_uri add-template 'https://webex.com<int24=123>/meeting<int16> room<int16>'```
- Encode a template

    ```./build/bin/numero_uri encode 'https://webex.com/meeting2/room56'```

- Decode a number string

    ```./build/bin/numero_uri decode 0x00000500020038000000000000000000```

- Remove a template

    ```./build/bin/numero_uri remove-template 123```

- Select a template file

    ```./build/bin/numero_uri config template-file /path/to/template/templates.json```

---
## Tests
### Docker
- ```make cmd='TEST'```

### Local
- `ctest --test-dir=build/tests`

---