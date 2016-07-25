// Copyright 2015-2016 the openage authors. See copying.md for legal info.

import QtQuick 2.4
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.3

import yay.sfttech.livereload 1.0
import yay.sfttech.openage 1.0

Item {
	id: root

	/*
	 * Global metric declaration.
	 */
	FontMetrics {
		id: fontMetrics
	}

	QtObject {
		id: metrics
		property real scale: 1
		property int unit: fontMetrics.averageCharacterWidth * scale
	}

	GameSpec {
		id: specObj

		/**
		 * States: Null, Loading, Ready
		 */

		// Start loading assets immediately
		active: true

		assetManager: amObj

		LR.tag: "spec"
	}

	AssetManager {
		id: amObj

		dataDir: MainArgs.dataDir

		LR.tag: "am"
	}

	GameMain {
		id: gameObj

		/**
		 * States: Null, Running
		 */

		engine: Engine

		LR.tag: "game"
	}

	GameControl {
		id: gameControlObj

		engine: Engine
		game: gameObj

		modes: [createModeObj, editorModeObj, actionModeObj]

		LR.tag: "gamecontrol"
	}

	GeneratorParameters {
		id: genParamsObj

		LR.tag: "gen"
	}

	ColumnLayout {
		id: controls

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top

		spacing: fontMetrics.averageCharacterWidth * 2

		state: gameControlObj.mode ? gameControlObj.mode.name : ""

		Component {
			id: changeMode

			ButtonFlat {
				text: "change_mode"
				onClicked: gameControlObj.modeIndex = (gameControlObj.effectiveModeIndex + 1) % gameControlObj.modes.length
			}
		}

		CreateMode {
			id: createModeObj
			LR.tag: "createMode"
		}

		ActionMode {
			id: actionModeObj
			LR.tag: "actionMode"
		}

		EditorMode {
			id: editorModeObj

			currentTypeId: typePicker.current
			currentTerrainId: typePicker.currentTerrain
			paintTerrain: typePicker.paintTerrain

			onToggle: typePicker.toggle()

			LR.tag: "editorMode"
		}

		states: [
			State {
				id: creationMode
				name: createModeObj.name

				property list<Item> content: [
					Loader {
						sourceComponent: changeMode
					},
					GeneratorParametersConfiguration {
						generatorParameters: genParamsObj
					},
					GeneratorControl {
						generatorParameters: genParamsObj
						gameSpec: specObj
						game: gameObj
					}
				]

				PropertyChanges {
					target: controls
					children: creationMode.content
					anchors.topMargin: fontMetrics.averageCharacterWidth * 4
					anchors.leftMargin: fontMetrics.averageCharacterWidth * 2
				}
			},
			State {
				id: editorMode
				name: editorModeObj.name

				property list<Item> content: [
					Loader {
						sourceComponent: changeMode
					},
					TypePicker {
						id: typePicker

						Layout.preferredWidth: root.width / 4
						Layout.preferredHeight: root.height / 2

						iconHeight: fontMetrics.averageCharacterWidth * 8

						editorMode: editorModeObj
						gameSpec: specObj
					},
					Text {
						color: "white"
						text: typePicker.currentHighlighted != -1 ? typePicker.currentHighlighted : ""
					}
				]

				PropertyChanges {
					target: controls
					children: editorMode.content
					anchors.topMargin: fontMetrics.averageCharacterWidth * 4
					anchors.leftMargin: fontMetrics.averageCharacterWidth * 2
				}
			},
			State {
				id: actionMode
				name: actionModeObj.name

				property list<Item> content: [
					IngameHud {
						anchors.fill: root

						game: gameObj
						actionMode: actionModeObj
						playerName: gameControlObj.currentPlayerName
						civIndex: gameControlObj.currentCivIndex
					}
				]

				PropertyChanges {
					target: controls; children: actionMode.content
				}
			}
		]
	}

	ColumnLayout {
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: fontMetrics.averageCharacterWidth * 4
		anchors.rightMargin: fontMetrics.averageCharacterWidth * 2

		spacing: fontMetrics.averageCharacterWidth * 2

		BindsHelp {
			Layout.alignment: Qt.AlignRight

			gameControl: gameControlObj
		}
	}

	Component.onCompleted: {
		ImageProviderByFilename.gameSpec = specObj
		ImageProviderById.gameSpec = specObj
		ImageProviderByTerrainId.gameSpec = specObj
	}
}
