Vue.component("loader", {
	template: "#loader-html"
});

var app = new Vue({
	el: '#app',
	data: function () {
		return {
			settings: {
				activeTab: 0,
				currentWifi: null,
				enterPasswordOpen: false,
				wifiLoading: false,
				countersLoading: false,
				password: "",
				current: null,
				wifis: [],
				counters: {
					value1: 0,
					value2: 0
				}
			}
		};
	},
	methods: {
		selectSettinsTab: function (val) {
			this.settings.activeTab = val;
			if (val === 0) {
				this.loadWifis();
			} else if (val === 1) {
				this.loadValues();
			}

		},
		openPasskeyForm: function (wifi) {
			this.settings.currentWifi = wifi;
			this.settings.enterPasswordOpen = true;
		},
		closePasskeyForm: function () {
			this.settings.enterPasswordOpen = false;
		},

		connect: function () {
			axios.post(
				"/api/wifi-connect",
				"ssid=" + encodeURIComponent(this.settings.currentWifi.ssid) + "&passkey=" + encodeURIComponent(this.settings.password),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					this.settings.password = "";
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.selectSettinsTab.bind(this, this.settings.activeTab), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		saveValues: function () {
			var val1 = String(this.settings.counters.value1).split("."),
				val2 = String(this.settings.counters.value2).split(".");

			axios.post(
				"/api/values-update",
				"val1=" + (val1[0] || 0) + "&dec1=" + (val1[1] || 0) + "&val2=" + (val2[0] || 0) + "&dec2=" + (val2[1] || 0),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.selectSettinsTab.bind(this, this.settings.activeTab), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		loadValues: function () {
			this.settings.countersLoading = true;
			axios.get("/api/values")
				.then(function (data) {
					if (data && data.data)
					{
						this.settings.counters.value1 = data.data.value1 + (data.data.decimals1 / 100);
						this.settings.counters.value2 = data.data.value2 + (data.data.decimals2 / 100);
					}
					this.settings.countersLoading = false;
				}.bind(this))
				.catch(function (err) {
					this.settings.countersLoading = false;
					alert(err);
				}.bind(this));

		},

		loadWifis: function () {
			this.settings.wifiLoading = true;
			axios.get("/api/wifi-list")
				.then(function (data) {
					this.settings.current = data.data.connected;
					this.settings.wifis.push.apply(this.settings.wifis, data.data.listAvailable);
					this.settings.wifiLoading = false;
				}.bind(this))
				.catch(function (err) {
					this.settings.wifiLoading = false;
					alert(err);
				}.bind(this));
		}
	},
	mounted: function() {
		this.selectSettinsTab(0);
	}
});