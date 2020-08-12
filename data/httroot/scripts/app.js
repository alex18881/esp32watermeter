Vue.component("loader", {
	template: "#loader-html"
});

Vue.component("settings-wifi-login", {
	template: "#settings-wifi-login-html",
	props: [
		'currentWifi'
	],
	data: function () {
		return {
			password: ""
		};
	},

	methods: {
		connect: function () {
			axios.post(
				"/api/wifi-connect",
				"ssid=" + encodeURIComponent(this.currentWifi.ssid) + "&passkey=" + encodeURIComponent(this.password),
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					this.password = "";
					this.$emit('close');
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},
		closeModal: function () {
			this.$emit('dismiss');
		}
	}
});

Vue.component("settings-page", {
	template: "#settings-page-html",
	data: function () {
		return {
			activeTab: 0
		};
	},
	methods: {
		selectSettinsTab: function (val) {
			this.activeTab = val;
		},

		resetSettings: function () {
			if (!confirm("Вы собираетесть сбросить все настройки. Продолжить?")) return;
			axios.post("/api/reset")
				.then(function () { alert("Сброшено"); })
				.catch(function (err) { alert(err); });
		}
	}
});

Vue.component("settings-wifi", {
	template: "#settings-wifi-html",
	data: function () {
		return {
			loading: false,
			current: null,
			wifis: [],
			password: "",
			enterPasswordOpen: false
		};
	},
	methods: {
		loadWifis: function () {
			this.loading = true;
			axios.get("/api/wifi-list")
				.then(function (data) {
					if (data.data.reload && data.data.time) {
						window.setTimeout(
							this.loadWifis.binf(this),
							data.data.time * 1000
						);
					} else {
						this.current = data.data.connected;
						this.wifis.push.apply(this.wifis, data.data.listAvailable);
						this.loading = false;
					}
				}.bind(this))
				.catch(function (err) {
					this.loading = false;
					alert(err);
				}.bind(this));
		},
		openPasskeyForm: function (wifi) {
			this.currentWifi = wifi;
			this.enterPasswordOpen = true;
		},
		closePasskeyForm: function (reload) {
			this.currentWifi = null;
			this.enterPasswordOpen = false;
			if (reload) {
				alert("Сохранено. Перезагрузка");
				window.setTimeout(this.loadWifis.bind(this), 10000);
			}
		}
	},
	mounted: function () {
		this.loadWifis();
	}
});

Vue.component("settings-counters", {
	template: "#settings-counters-html",
	data: function () {
		return {
			value1: 0,
			value2: 0,
			tickValue1: 0,
			tickValue2: 0,
			loading: false
		};
	},
	methods: {
		saveValues: function () {
			axios.post(
				"/api/values-update",
				"val1=" + (this.value1 || 0) + "&val2=" + (this.value2 || 0) + "&ticks1=" + this.tickValue1 + "&ticks2=" + this.tickValue2,
				{ headers: { 'content-type': 'application/x-www-form-urlencoded' } }
			)
				.then(function () {
					alert("Сохранено. Перезагрузка");
					window.setTimeout(this.loadValues.bind(this), 10000);
				}.bind(this))
				.catch(function (err) {
					alert(err);
				}.bind(this))
		},

		loadValues: function () {
			this.loading = true;
			axios.get("/api/values")
				.then(function (data) {
					if (data && data.data) {
						this.value1 = data.data.value1;
						this.value2 = data.data.value2;
						this.tickValue1 = data.data.ticks1;
						this.tickValue2 = data.data.ticks2;
					}
					this.loading = false;
				}.bind(this))
				.catch(function (err) {
					this.loading = false;
					alert(err);
				}.bind(this));
		}
	},
	mounted: function () {
		this.loadValues();
	}
});

Vue.component("counters-page", {
	template: "#counters-page-html",
	data: function () {
		return {
			loading: false,
			value1: 0,
			value2: 0,
			decimals1: 0,
			decimals2: 0,
			loadingTimer: null
		}
	},
	methods: {
		applyData: function (data) {
			if (this._isDestroyed || this._isBeingDestroyed) return;
			
			this.loading = false;
			this.value1 = ('000000' + data.data.value1).slice(0, -3).slice(-5).split("");
			this.value2 = ('000000' + data.data.value2).slice(0, -3).slice(-5).split("");
			this.decimals1 = ('000' + data.data.value1).slice(-3).split("");
			this.decimals2 = ('000' + data.data.value2).slice(-3).split("");
			this.loadingTimer = window.setTimeout(this.loadValues.bind(this), 5000);
		},

		loadValues: function () {
			if (this.loadingTimer) {
				window.clearTimeout(this.loadingTimer);
			}
			axios.get("/api/values")
				.then(this.applyData.bind(this))
				.catch(function (err) {
					this.loading = false;
					alert(err);
				}.bind(this));
		}
	},

	mounted: function () {
		this.loading = true;
		this.loadValues()
	},

	destroyed: function () {
		if (this.loadingTimer) {
			window.clearTimeout(this.loadingTimer);
		}
	}
});

var app = new Vue({
	el: '#app',
	data: function () {
		return {
			page: 0,
			menuVisible: false
		};
	},
	methods: {
		toggleMenu: function () {
			this.menuVisible = !this.menuVisible
		},
		openPage: function(index) {
			this.menuVisible = false;
			this.page = index;
		}
	},
	mounted: function() {
	}
});