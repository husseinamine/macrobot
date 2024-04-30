window.addEventListener('load', function() {
    let blocked = false;
    if (!!window.EventSource) {
        var source = new EventSource('/events');

        source.addEventListener('open', function(e) {
            console.log("Events Connected")
        }, false);

        source.addEventListener('error', function(e) {
            if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected")
            }
        }, false);

        source.addEventListener('blocked', function(e) {
            console.log("blocked: ", e.data)
        }, false);
    }

    var websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
    websocket.onopen = function(event) {
    console.log('Connection established');
    }
    websocket.onclose = function(event) {
    console.log('Connection died');
    }
    websocket.onerror = function(error) {
    console.log('error');
    };
    websocket.onmessage = function(event) {
    if (event.data == "blocked") {
        blocked = true
    }
    else if (event.data == "remove-blocked") {
        blocked = false
    }
    };
    
    const sendToServer = (command) => {
        console.log(command)
        websocket.send('end')
        websocket.send(command)
    }

    const commands = ["forward", "backward", "turn-right", "turn-left"]
    let commandStart, commandEnd
    const addCommandListener = (command) => {
        const start = (e) =>  {
            if (document.getElementById(command).style.backgroundColor == "rgb(105, 105, 105)") return
            document.getElementById(command).style = `padding: 0px`

            let tempMacro = JSON.parse(window.localStorage.getItem("tempMacro"))
            if (tempMacro) {
                commandStart = new Date().getTime()
            }

            sendToServer(`${command}`)

            commands.forEach(c => {
                if (command != c) {
                    setDisabled(c)
                }
            })
        }
        
        const end = (e) => {
            if (document.getElementById(command).style.backgroundColor == "rgb(105, 105, 105)") return
            document.getElementById(command).style = `padding: 4px`

            let tempMacro = JSON.parse(window.localStorage.getItem("tempMacro"))
            if (tempMacro) {
                commandEnd = new Date().getTime()
                window.localStorage.setItem("tempMacro", JSON.stringify([...tempMacro, {command, t: commandEnd - commandStart}]))
            }

            sendToServer(`end`)
            commands.forEach(c => {
                if (command != c) {
                    setEnabled(c)
                }
            })
        }
        if (window.navigator.platform != "Win32") {
            document.getElementById(command).addEventListener('touchstart', start)
            document.getElementById(command).addEventListener('touchend', end)
        } else {
            document.getElementById(command).addEventListener('mousedown', start)
            document.getElementById(command).addEventListener('mouseup', end)
        }
    }

    const addControlListener = (control, fs, fe) => {
        if (window.navigator.platform != "Win32") {
            document.getElementById(control).addEventListener('touchstart', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                document.getElementById(control).style = `border-width: 0px`
                if (fs != null) {fs()}
            })
            document.getElementById(control).addEventListener('touchend', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                document.getElementById(control).style = `border-width: 4px`
                if (fe != null) {fe()}
            })
        } else {
            document.getElementById(control).addEventListener('mousedown', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                document.getElementById(control).style = `border-width: 0px`
                if (fs != null) {fs()}
            })
            document.getElementById(control).addEventListener('mouseup', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                document.getElementById(control).style = `border-width: 4px`
                if (fe != null) {fe()}
            })
        }
    }


    const addAppendedControlListener = (control, fs, fe) => {
        if (window.navigator.platform != "Win32") {
            control.addEventListener('touchstart', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                control.style = `border-width: 0px`
                if (fs != null) {fs()}
            })
            control.addEventListener('touchend', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                control.style = `border-width: 4px`
                if (fe != null) {fe()}
            })
        } else {
            control.addEventListener('mousedown', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                control.style = `border-width: 0px`
                if (fs != null) {fs()}
            })
            control.addEventListener('mouseup', e => {
                if (window.localStorage.getItem(`${control}-style`) != null) return
                control.style = `border-width: 4px`
                if (fe != null) {fe()}
            })
        }
    }

    let styles = {}
    const setDisabled = (control) => {
        window.localStorage.setItem(`${control}-style`, document.getElementById(control).style.all)
        document.getElementById(control).style = "background-color: rgb(105, 105, 105);color: whitesmoke;"
    }

    const setEnabled = (control) => {
        document.getElementById(control).style = window.localStorage.getItem(`${control}-style`)
        window.localStorage.removeItem(`${control}-style`)
    }

    addCommandListener("forward")
    addCommandListener("turn-right")
    addCommandListener("turn-left")
    addCommandListener("backward")

    addControlListener("start", null, () => {
        window.localStorage.setItem("tempMacro", "[]")

        setDisabled("start")
        setEnabled("stop")
    })
    addControlListener("stop", null, () => {
        let macroIndex = parseInt(window.localStorage.getItem("macroIndex")) + 1

        let macros = JSON.parse(window.localStorage.getItem("macros"))
        let tempMacro = JSON.parse(window.localStorage.getItem("tempMacro"))
        console.log(tempMacro.length)
        if (tempMacro.length > 0) {
            window.localStorage.setItem("macroIndex", macroIndex)
            window.localStorage.setItem("macros", JSON.stringify([...macros, {
                id: macroIndex, 
                macro: JSON.stringify(tempMacro)
            }]))
        }

        window.localStorage.removeItem("tempMacro")
        setDisabled("stop")
        setEnabled("start")

        updateMacros()
    })
    setDisabled("stop")
    setEnabled("start")
    commands.forEach(c => {
        setEnabled(c)
    })

    if (!window.localStorage.getItem("macros")) {
    window.localStorage.setItem("macros", "[]")
    }
    window.localStorage.setItem("macroIndex", JSON.parse(window.localStorage.getItem("macros")).length)

    const updateMacros = () => {
        let macros = JSON.parse(window.localStorage.getItem("macros"))
        let macrosDiv = document.getElementById("macros")
        macrosDiv.innerHTML = ""
        macros.forEach(macro => {
            const macroDiv = document.createElement('div')
            macroDiv.className = "macro"
            macroDiv.id = `macro${macro.id}`
            macroDiv.innerHTML = `
            <h1 style="color: white;padding-left: 15px;padding-top: 7.5px;">ma${macro.id}</h1>
            `

            const macroButtons = document.createElement("div")
            macroButtons.className = "macro-btns"
            macroButtons.style.paddingRight = "15px"

            const playButton = document.createElement("button")
            playButton.className = "macro-start"
            playButton.id = `macro-btn-${macro.id}`
            playButton.textContent = "PLAY"

            const deleteButton = document.createElement("button")
            deleteButton.className = "macro-del"
            deleteButton.textContent = "DELETE"

            addAppendedControlListener(playButton, null, () => {
                playMacro(macro.id)
            })

            addAppendedControlListener(deleteButton, null, () => {
                deleteMacro(macro.id)
            })

            macroButtons.appendChild(playButton)
            macroButtons.appendChild(deleteButton)
            macroDiv.appendChild(macroButtons)

            document.getElementById("macros").appendChild(macroDiv)
        })
    }

    const deleteMacro = (id) => {
        let macros = JSON.parse(window.localStorage.getItem("macros"))
        macros = macros.filter(macro => macro.id != id)
        window.localStorage.setItem("macros", JSON.stringify(macros))
        updateMacros()
    }

    const playMacro = (id) => {
        console.log("Test")
        if (window.localStorage.getItem(`macro-btn-${id}-style`) != null) return // not disabled 
        console.log("Test2")
        let macros = JSON.parse(window.localStorage.getItem("macros"))
        let macro = macros.filter(macro => macro.id == id)[0]
        
        let steps = macro.macro
        console.log(steps)

        setDisabled(`macro-btn-${id}`)
        const serverUrl = `http://192.168.4.1/playmacro`

        fetch(serverUrl, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded',
            },
            body: 'macro=' + encodeURIComponent(steps)
        })
        .then(response => {
            if (!response.ok) {
            throw new Error('Network response was not ok')
            }
            console.log(response.text())
        })
        .then(data => {
            console.log('Response from server:', data)
        })
        .catch(error => {
            console.error('Error sending command:', error)
        })
        .finally(() => setEnabled(`macro-btn-${id}`))
        
    }

    updateMacros()
})