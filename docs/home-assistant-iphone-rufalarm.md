# iPhone notifications for BrauKnecht Rufalarm

This guide sends one normal iPhone notification for each BrauKnecht `Rufalarm`. The delivery path is controller firmware → MQTT broker → Home Assistant automation → Home Assistant Companion app/APNs. It does not depend on browser JavaScript or on keeping the BrauKnecht web page open.

## Prerequisites

- Home Assistant's existing MQTT integration is connected to the same broker as BrauKnecht. Do not add a duplicate integration.
- The controller's existing MQTT host, port, and credentials are configured at [BrauKnecht `/config`](/config).
- The official Home Assistant Companion app is installed on the target iPhone and signed into this Home Assistant instance.
- In **iOS Settings → Notifications → Home Assistant**, enable **Allow Notifications**, **Sounds**, and **Lock Screen**.
- You may leave Background App Refresh enabled. Delivery is APNs-based and does not require Safari or the controller page to remain open.

This uses the ordinary iOS `default` notification sound. It does not bypass Silent Mode or Focus, and it does not request the critical-alert entitlement. Notification Summary, ringer volume, and the app's per-notification sound setting can also defer or suppress sound.

## Identify your controller and iPhone action

BrauKnecht publishes the non-retained event topic:

```
brauknecht/<nodeId>/rufalarm
```

Find `<nodeId>` in an existing MQTT state or discovery topic. For example, for state topic `brauknecht/brauknecht_a1b2c3/state`, use `brauknecht_a1b2c3`. The firmware defines this topic prefix; there is no MQTT base-topic setting to configure. The event payload is exactly `Rufalarm` and it is not retained.

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

Create one enabled automation with this YAML, replacing only `<nodeId>` and `<iphone_action_suffix>`:

```yaml
alias: BrauKnecht Rufalarm iPhone
id: brauknecht_rufalarm_iphone
triggers:
  - trigger: mqtt
    topic: "brauknecht/<nodeId>/rufalarm"
    payload: "Rufalarm"
conditions: []
actions:
  - action: notify.mobile_app_<iphone_action_suffix>
    data:
      title: "BrauKnecht"
      message: "Rufalarm"
      data:
        push:
          sound: default
mode: queued
max: 20
```

The UI is equivalent: go to **Settings → Automations & scenes → Create automation → Create new automation**. Add an MQTT trigger with the exact topic and payload, then add the discovered `notify.mobile_app_*` action. If the visual editor does not expose nested `data.push.sound: default`, switch only that action editor to YAML. Set automation mode to **Queued** with a maximum of `20`, save, and enable it.
