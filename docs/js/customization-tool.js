$(document).ready(function () {
    //Handles popover click
    $('[data-toggle="popover"]').popover();  

    /**
     * Handle file import
     */
    $('#file-selector').on('change', function (oEvent) {
        showLoading();
        if (window.FileReader) {
            const oFile = oEvent.target.files[0];
            const oReader = new FileReader();

            oReader.onload = function () {
                // Empty standard expression and settings array
                aExpressions.length = 0;
                aSettings.length = 0;

                // Reduce file content to expression declaration 
                var sFileContent = oReader.result;
                const iExpressionDeclarationStartIndex = sFileContent.indexOf('//CUSTOM_EXPRESSION_DECLARATION_START') + 37;
                const iExpressionDeclarationEndIndex = sFileContent.indexOf('//CUSTOM_EXPRESSION_DECLARATION_END');
                const sExpressionDeclaration = sFileContent.substring(iExpressionDeclarationStartIndex, iExpressionDeclarationEndIndex);

                // Extract expressions
                const aExtractedExpressions = [...sExpressionDeclaration.matchAll('"(.*?)"')];
                aExtractedExpressions.forEach(sExpression => {
                    aExpressions.push(sExpression[1]);
                });

                // Reduce file content to settings declaration
                sFileContent = oReader.result;
                const iSettingsDeclarationStartIndex = sFileContent.indexOf('//CUSTOM_SETTINGS_DECLARATION_START') + 36;
                const iSettingsDeclarationEndIndex = sFileContent.indexOf('//CUSTOM_SETTINGS_DECLARATION_END') - 1;
                const sSettingsDeclaration = sFileContent.substring(iSettingsDeclarationStartIndex, iSettingsDeclarationEndIndex);

                // Extraction settings
                aExtractedSettingsByLines = sSettingsDeclaration.split('\n');
                aExtractedSettingsByLines.forEach(sSettingLine => {
                    aSettings.push(sSettingLine.split(' ')[2])
                });

                // Inform user about successful upload
                if (aExpressions.length === 128 && aSettings.length === 9) {
                    hideLoading();
                    updateSettingsElements();
                    alert('Upload was successful. You can now edit your expressions.');
                } else {
                    console.error('aExpression length ' + aExpressions.length);
                    console.error(aExpressions);
                    console.error('aSettings length ' + aSettings.length);
                    console.error(aSettings);
                    hideLoading();
                    alert('Your upload file, seems incorrect. Make sure you haven\'t edited the file between "//EXPRESSION_DECLARATION_START" and "//EXPRESSION_DECLARATION_END". Try to start with a fresh version, by downloading a new customization.');
                }
            }
            oReader.onerror = function () {
                alert('The file could not be read: ' + oReader.error + '. Try it with another browser or conatct us.');
            };

            try {
                oReader.readAsText(oFile);
            } catch (error) {
                hideLoading();
                alert("No file chosen. Please select a file in the dialog");
            }
            
        } else {
            alert("Your browser does not support a file upload. Please use a modern browser for it.");
        }
    });

    /**
     * Handle open expression modal clicks dynamically
     */
    $('.open-expression-modal').on('click', function () {
        const iProfile = $(this).attr('data-profile');
        const sDirection = $(this).attr('data-direction');
        const aRelevantExpressions = getExpressions(iProfile, sDirection);

        // Set textfield values
        $("#expression-input-left").val(aRelevantExpressions[0]);
        $("#expression-input-up").val(aRelevantExpressions[1]);
        $("#expression-input-right").val(aRelevantExpressions[2]);
        $("#expression-input-down").val(aRelevantExpressions[3]);

        // Set modal header and save button data attributes
        manipulateModal(iProfile, sDirection);

        //Open modal expressionsModal
        $("#expressionsModal").modal();
    });

    /**
     * Handle save button in expression modal
     */
    $('#expression-modal-save').on('click', function () {
        const iProfile = $(this).attr('data-profile');
        const sDirection = $(this).attr('data-direction');
        var sExpressionInputLeft = $("#expression-input-left").val();
        var sExpressionInputUp = $("#expression-input-up").val();
        var sExpressionInputRight = $("#expression-input-right").val();
        var sExpressionInputDown = $("#expression-input-down").val();
        const aNewExpressions = [sExpressionInputLeft, sExpressionInputUp, sExpressionInputRight, sExpressionInputDown];

        //Delete all special chracters outside a-z and A-Z
        aNewExpressions.forEach(function (part, index) {
            this[index] = this[index].replace(/[^a-zA-Z ]/g, '');
        }, aNewExpressions);

        //Update expressions
        setExpressions(aNewExpressions, iProfile, sDirection);

        //Close modal
        $("#expressionsModal").modal('hide');
    });

    /**
     * Handle code download
     */
    $("#btnDownloadCode").click(function () {

        const iTotalNumberOfCharacters = getTotalNumberOfCharactersInExpressions();

        //Check total length for Arduino memory reasons
        if (iTotalNumberOfCharacters > 15000) {
            alert('You used to long strings in your profiles. Allowed number of characters in all expressions combined: 15,000. You used: ' + iTotalNumberOfCharacters + ' characters. Please reduce the length of used expressions in your profiles.');
        } else {
            // Save settings
                //TODO

            //Get Arduino Code Template
            var oRequest = new XMLHttpRequest();
            oRequest.addEventListener("load", codeRequestLoad);
            oRequest.open("GET", "/Speak4me/assets/speak4me.ino", true);
            oRequest.send();

        }
    });

    /**
     * Handle PDF download
     */
    // TODO: Create "rectangle strucure" instead of table, see: https://rawgit.com/MrRio/jsPDF/master/
    $("#btnPdfDownload").on('click', function () {
        var oDocument = new jsPDF();
        const iFinalY = oDocument.lastAutoTable.finalY || 10;
        const aTableContent = createTableContent();

        oDocument.text('Profile overview', 14, iFinalY + 15)
        oDocument.setFontSize(10);
        oDocument.text('Below is a listing of the standard as well as your custom expressions and the corresponding direction.', 14, iFinalY + 20);
        oDocument.autoTableSetDefaults({
            headStyles: {
                fillColor: [255, 165, 0]
            },
        });
        oDocument.autoTable({
            startY: iFinalY + 25,
            head: [
                ['Profile', '1. Direction', '2. Direction', 'Expression', 'Explanation']
            ],
            body: aTableContent
        });

        oDocument.save("Your-Profile-Expression-Table.pdf");
    });

    /**
     * Sets the title of the expressions-modal dynamically
     * @param {number} iProfile Profile number e.g. 1
     * @param {string} sDirection Direction e.g. Left
     */
    function manipulateModal(iProfile, sDirection) {
        $('#modal-expression-title').text('Profile ' + iProfile + ' - ' + sDirection);
        $('#expression-modal-save').attr('data-profile', iProfile);
        $('#expression-modal-save').attr('data-direction', sDirection);
    }

    /**
     * Returs an array of appropriate expressions based on profile and direction
     * @param {number} iProfile Profile number e.g. 1
     * @param {string} sDirection Direction e.g. Left
     * @returns {Array} The 4 expressions based on profile and direction
     */
    function getExpressions(iProfile, sDirection) {
        const iExpressionsStartIndex = getExpressionsIndexStart(iProfile, sDirection);
        return aExpressions.slice(iExpressionsStartIndex, iExpressionsStartIndex + 5);
    }

    /**
     * Returns start index of appropirate expressions based on profile and direction
     * @param {number} iProfile Profile number e.g. 1
     * @param {string} sDirection Direction e.g. Left
     * @returns {number} Index of the first expression based on profile and direction
     */
    function getExpressionsIndexStart(iProfile, sDirection) {
        var iStartOfProfile = null;
        var iStartOfDirection = null;

        //Calculate the index of the array, based on profile id
        iStartOfProfile = iProfile * 16 - 16;

        //Calculate the index of the array, based on the direction
        switch (sDirection) {
            case 'Left':
                iStartOfDirection = 0;
                break;
            case 'Up':
                iStartOfDirection = 4;
                break;
            case 'Right':
                iStartOfDirection = 8;
                break;
            case 'Down':
                iStartOfDirection = 12;
                break;
            default:
                break;
        }
        return iExpressionsStartIndex = iStartOfProfile + iStartOfDirection;
    }

    /**
     * Updates aExpression with new expressions for appropriate profile and direction
     * @param {Array} aNewExpressions Contains new expressions in the order: left, up, right, down
     * @param {number} iProfile Profile number e.g. 1
     * @param {string} sDirection Direction e.g. Left
     */
    function setExpressions(aNewExpressions, iProfile, sDirection) {
        const iExpressionsStartIndex = getExpressionsIndexStart(iProfile, sDirection);
        aExpressions.splice(iExpressionsStartIndex, aNewExpressions.length, ...aNewExpressions);
    }

    /**
     * Calculates the toral length of all strings in aExpressions
     * @returns {number} Length of all combined strings in aExpressions
     */
    function getTotalNumberOfCharactersInExpressions() {
        var totalLength = 0;
        aExpressions.forEach(function (sItem) {
            totalLength += sItem.length;
        })
        return totalLength;
    }

    /**
     * Event listener for Arduino template code request laod
     */
    function codeRequestLoad() {
        var sArduinoCode = this.response;
        saveSettingsInputs();

        // Insert expressions into code
        $.each(aExpressions, function (key, value) {
            sArduinoCode = sArduinoCode.replace('CUSTOM_EXPRESSION_' + key, value)
        });

        // Insert settings options into the code
        $.each(aSettings, function (key, value) {
            sArduinoCode = sArduinoCode.replace('CUSTOM_SETTING_' + key, value)
        });

        // Download file
        downloadFile('speak4me.ino', sArduinoCode);
    }

    /**
     * Download a file
     * @param {string} sFilename Name of the file that will be downloaded
     * @param {string} sFileContent Content of the file
     */
    function downloadFile(sFilename, sFileContent) {
        var element = document.createElement('a');
        element.setAttribute('href', 'data:text/plain;charset=utf-8,' + encodeURIComponent(sFileContent));
        element.setAttribute('download', sFilename);
        element.style.display = 'none';
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    }
    /**
     * @returns {Array} Multi-dimensional array that contains all expressions including, profile name and direction as well a explanation for each expression
     */
    function createTableContent() {
        var aTableContent = [];
        var iProfile = 1;
        var sFirstDirection = 'Left';
        var sSecondDirection = 'Left';
        const aDirections = ['Left', 'Up', 'Right', 'Down'];
        var iFirstDirectionIndex = 0;
        var iSecondDirectionIndex = 0;

        // Add main profile expressions
        aTableContent.push(['Main Profile', 'Left', 'Left', 'Leave Menu', 'Will leave the menu profile.']);
        aTableContent.push(['Main Profile', 'Left', 'Up', 'Volume Up', 'Will increase the volume of the speaker.']);
        aTableContent.push(['Main Profile', 'Left', 'Right', 'Toggle Mute', 'Mute or unmute the Speak4Me glass']);
        aTableContent.push(['Main Profile', 'Left', 'Down', 'Volume Down', 'Will decrease the volume of the speaker.']);
        aTableContent.push(['Main Profile', 'Up', 'Left', 'No', '']);
        aTableContent.push(['Main Profile', 'Up', 'Up', 'Okay', '']);
        aTableContent.push(['Main Profile', 'Up', 'Right', 'Yes', '']);
        aTableContent.push(['Main Profile', 'Up', 'Down', 'Help', '']);
        aTableContent.push(['Main Profile', 'Right', 'Left', 'Profile 4', 'Switch to profile 4']);
        aTableContent.push(['Main Profile', 'Right', 'Up', 'Profile 1', 'Switch to profile 1']);
        aTableContent.push(['Main Profile', 'Right', 'Right', 'Profile 2', 'Switch to profile 2']);
        aTableContent.push(['Main Profile', 'Right', 'Down', 'Profile 3', 'Switch to profile 3']);
        aTableContent.push(['Main Profile', 'Right', 'Left', 'Profile 8', 'Switch to profile 8']);
        aTableContent.push(['Main Profile', 'Right', 'Up', 'Profile 5', 'Switch to profile 5']);
        aTableContent.push(['Main Profile', 'Right', 'Right', 'Profile 6', 'Switch to profile 6']);
        aTableContent.push(['Main Profile', 'Right', 'Down', 'Profile 7', 'Switch to profile 7']);

        // Add custom profiles 1-8
        aExpressions.forEach(function (sExpression, iIndex) {
            // Increase profile
            if (iIndex % 16 === 0 && iIndex != 0) {
                iProfile++;
            }

            // Change first direction
            if (iIndex % 4 === 0 && iIndex != 0) {
                iFirstDirectionIndex++;
                if (iFirstDirectionIndex === 4) {
                    iFirstDirectionIndex = 0;
                }
                sFirstDirection = aDirections[iFirstDirectionIndex]
            }

            // Change second direction 
            if (iSecondDirectionIndex === 4) {
                iSecondDirectionIndex = 0;
            }
            sSecondDirection = aDirections[iSecondDirectionIndex];
            iSecondDirectionIndex++;

            // Add new element
            aTableContent.push(['Profile ' + iProfile, sFirstDirection, sSecondDirection, sExpression, '']);
        });

        return aTableContent;
    }

    /**
     * Show upload busy indicator
     */
    function showLoading() {
        document.getElementById("loader").style.display = "block";
        document.getElementById("loader-frame").style.display = "block";
        document.getElementById("loading").style.display = "block";

    }

    /**
     * Hide upload busy indicator
     */
    function hideLoading() {
        document.getElementById("loader").style.display = "none";
        document.getElementById("loading").style.display = "none";
        document.getElementById("loader-frame").style.display = "none";
    }

    /**
     * Updates the settings inputs based on the aSettings array (standard or updates from file import)
     */
    function updateSettingsElements(){
        document.getElementById("voiceStyle").value = aSettings[0];
        document.getElementById("initialVolume").value = aSettings[1];
        document.getElementById("initialVolumeOutput").value = aSettings[1];
        document.getElementById("closedDuration").value = aSettings[2];
        document.getElementById("keepAliveOpt").value = aSettings[3];
        document.getElementById("statusLEDActive").value = aSettings[4];
        document.getElementById("dirDuration").value = aSettings[5];
        document.getElementById("dirPause").value = aSettings[6];
        document.getElementById("timeOutDuration").value = aSettings[7];
        document.getElementById("calibration").value = aSettings[8];
    }

    /**
     * Stores input values in aSettings array
     */
    function saveSettingsInputs(){
        aSettings[0] = document.getElementById("voiceStyle").value;
        aSettings[1] = document.getElementById("initialVolume").value;
        aSettings[2] = document.getElementById("closedDuration").value;
        aSettings[3] = document.getElementById("keepAliveOpt").value;
        aSettings[4] = document.getElementById("statusLEDActive").value;
        aSettings[5] = document.getElementById("dirDuration").value;
        aSettings[6] = document.getElementById("dirPause").value;
        aSettings[7] = document.getElementById("timeOutDuration").value;
        aSettings[8] = document.getElementById("calibration").value;
    }

    // Contains all custom expressions (excluding main profile expressions) in a ASC order of profiles and a left,up,right, order in each profile
    var aExpressions = [
        // Profile 1
        "I want to wash myself",
        "I need to go to the toilet",
        "I would like to brush my teeth",
        "I would like to change my clothes",
        "Okay",
        "Yes",
        "Help",
        "No",
        "I want time for myself",
        "I want to sleep",
        "I need a break",
        "I would like to stop",
        "It is too hot",
        "I am hungry",
        "I am thirsty",
        "I want sweets",
        // Profile 2
        "I dont like this",
        "I am good, thanks",
        "I am feeling not so good today",
        "I am tired",
        "Okay",
        "Yes",
        "Help",
        "No",
        "Do you want to hang out?",
        "How are you?",
        "What are your plans for the day??",
        "Want to go for a walk?",
        "Nice",
        "That is wonderful",
        "Oh no",
        "Hahaha",
        // Profile 3
        "Tighten my shoe",
        "I am freezing, I need warmer clothes",
        "I am warm, I need less clothes",
        "Change my clothes",
        "Okay",
        "Yes",
        "Help",
        "No",
        "I want to lay down",
        "Turn me",
        "Scratch me",
        "I want to sit",
        "I need a massage",
        "I need a medical treatment",
        "I am in pain",
        "Something is wrong",
        // Profile 4
        "I have a problem",
        "I dont feel well today",
        "I feel good today",
        "I am in pain",
        "Okay",
        "Yes",
        "Help",
        "No",
        "Ask me which part of the body is affected",
        "Something is wrong",
        "I need medication",
        "I need treatment",
        "Left",
        "Higher",
        "Right",
        "Lower",
        // Profile 5
        "8",
        "5",
        "6",
        "7",
        "4",
        "1",
        "2",
        "3",
        "D",
        "A",
        "B",
        "C",
        "H",
        "E",
        "F",
        "G",
        // Profile 6
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        // Profile 7
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        // Profile 8
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression",
        "Feel free to set your own expression"

    ];

    // Contains all custom settings
    var aSettings = [
        "1", // voiceStyle Male or female voice
        "4", //initialVolume Update Volume Funktion einbinden, Aus dem Customizationtool kommt 1-5
        "2000", // closedDuration How long to close the eyes to open the menu
        "true", // keepAliveOpt true/false
        "true", //statusLEDActive Learning LED which turns on when a direction is detected 
        "500", // dirDuration Duration for which a direction has to be measured to be recognized
        "1000", // dirPause Cooldown after dir1 has been recognized
        "4000", // timeOutDuration Timelimit after which direction 1 gets rejected
        "true"  // useAutoCalibration true/false 
    ]

});