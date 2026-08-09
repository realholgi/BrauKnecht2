# iPhone notifications for BrauKnecht Rufalarm

This guide sends one normal iPhone notification for each BrauKnecht Rufalarm, including its operator-visible cause. The delivery path is controller firmware → MQTT broker → Home Assistant automation → Home Assistant Companion app/APNs. It does not depend on browser JavaScript or on keeping the BrauKnecht web page open.

## Prerequisites

- Home Assistant's existing MQTT integration is connected to the same broker as BrauKnecht. Do not add a duplicate integration.
- The controller's existing MQTT host, port, and credentials are configured at [BrauKnecht `/config`](/config).
- The official Home Assistant Companion app is installed on the target iPhone and signed into this Home Assistant instance.
- In **iOS Settings → Notifications → Home Assistant**, enable **Allow Notifications**, **Sounds**, and **Lock Screen**.
- You may leave Background App Refresh enabled. Delivery is APNs-based and does not require Safari or the controller page to remain open.

Normal Rufalarm causes use the ordinary iOS `default` notification sound. Notification Summary, ringer volume, Focus, and the app's per-notification sound setting can defer or suppress those normal notifications.

For a `sensor_fault` critical alert, the Home Assistant Companion app must have **Critical Alerts** enabled in **iOS Settings → Notifications → Home Assistant**. Critical alerts may bypass Silent Mode and Focus; use them only for the sensor-fault condition that shuts heating down.

## Identify your controller and iPhone action

BrauKnecht publishes the non-retained event topic:

```
brauknecht/<nodeId>/rufalarm
```

Find `<nodeId>` in an existing MQTT state or discovery topic. For example, for state topic `brauknecht/brauknecht_a1b2c3/state`, use `brauknecht_a1b2c3`. The firmware defines this topic prefix; there is no MQTT base-topic setting to configure.

The event is non-retained. Its payload is one stable cause identifier:

| Payload | Meaning |
| --- | --- |
| `mash_start` | The mash-in target was reached. |
| `rest_complete` | A timed mash rest completed. |
| `mash_end` | The mash-out target was reached. |
| `boil_end` | The boil timer completed. |
| `sensor_fault` | A temperature sensor fault was detected. |
| `alarm_test` | The controller's alarm test was started. |

The MQTT state JSON also includes `alarm_reason` and `alarm_action`. `alarm_action` is `acknowledge_at_controller` for normal alarms and `check_sensor_and_acknowledge_at_controller` for `sensor_fault`. Neither MQTT nor the web dashboard provides remote acknowledgement: acknowledge the alarm physically at the controller.

To identify the iPhone notification action, open **Developer Tools → Actions** in Home Assistant, search for `notify.mobile_app_`, select the action for the target iPhone, and copy its exact name. Its suffix after `notify.mobile_app_` is `<iphone_action_suffix>`. Do not derive it from the iPhone display name: renaming the phone or Companion app device can change the action name.

Before creating the automation, select that action in **Developer Tools → Actions** and send this direct test while the iPhone is locked:

```yaml
title: "BrauKnecht"
message: "Rufalarm"
data:
  push:
    sound: default
```

Confirm the notification reaches the lock screen before troubleshooting MQTT. Sound occurs only when the iPhone's ringer, volume, Focus, scheduled summary, and Home Assistant notification settings permit it.

## Create the automation

Create one enabled automation with this YAML, replacing only `<nodeId>` and `<iphone_action_suffix>`. The `sensor_fault` MQTT event gets a critical iPhone alert; all other causes remain normal notifications.

```yaml
alias: BrauKnecht Rufalarm iPhone
id: brauknecht_rufalarm_iphone
triggers:
  - trigger: mqtt
    topic: "brauknecht/<nodeId>/rufalarm"
conditions: []
actions:
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ trigger.payload == 'sensor_fault' }}"
        sequence:
          - action: notify.mobile_app_<iphone_action_suffix>
            data:
              title: "BrauKnecht sensor fault"
              message: "Temperature sensor fault — heating is off. Check the sensor and acknowledge at the controller."
              data:
                push:
                  sound:
                    name: default
                    critical: 1
                    volume: 1.0
    default:
      - action: notify.mobile_app_<iphone_action_suffix>
        data:
          title: "BrauKnecht"
          message: >-
            {{ {
              'mash_start': 'Mash-in target reached — acknowledge at the controller.',
              'rest_complete': 'Mash rest complete — acknowledge at the controller.',
              'mash_end': 'Mash-out target reached — acknowledge at the controller.',
              'boil_end': 'Boil complete — acknowledge at the controller.',
              'alarm_test': 'Alarm test — acknowledge at the controller.'
            }.get(trigger.payload, 'Rufalarm — acknowledge at the controller.') }}
          data:
            push:
              sound: default
mode: queued
max: 20
```

The UI is equivalent: go to **Settings → Automations & scenes → Create automation → Create new automation**. Add an MQTT trigger with the exact topic and no payload filter, then add a `Choose` action. Its `sensor_fault` branch calls the discovered `notify.mobile_app_*` action with `push.sound.critical: 1`; its default branch sends the normal reason-specific notification. If the visual editor does not expose the template, `choose` block, or nested push sound data, switch only the action editor to YAML. Set automation mode to **Queued** with a maximum of `20`, save, and enable it.
