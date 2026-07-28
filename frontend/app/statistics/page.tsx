"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select"
import { Activity, BarChart3, DollarSign, TrendingUp, PieChart, LineChart } from "lucide-react"
import { motion } from "framer-motion"
import {
  LineChart as RechartsLineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  BarChart as RechartsBarChart,
  Bar,
  PieChart as RechartsPieChart,
  Cell,
  Pie,
  AreaChart,
  Area,
  ScatterChart,
  Scatter,
  ComposedChart,
} from "recharts"

interface Statistics {
  orderbook: {
    bid_levels: number
    ask_levels: number
    bid_orders: number
    ask_orders: number
    total_orders: number
    total_trades: number
  }
  market_data: {
    best_bid: number
    best_ask: number
    spread: number
  }
  trades: {
    total_trades: number
    total_volume: number
    total_value: number
    avg_price: number
    min_price: number
    max_price: number
    last_trade_time: number
  }
}

// Mock data for charts - in real app, this would come from API
const generatePriceData = () => {
  const data = []
  let price = 150
  for (let i = 0; i < 24; i++) {
    price += (Math.random() - 0.5) * 5
    data.push({
      time: `${i}:00`,
      price: Number(price.toFixed(2)),
      volume: Math.floor(Math.random() * 1000) + 100,
    })
  }
  return data
}

const generateVolumeData = () => {
  return [
    { range: "0-100", count: 45, percentage: 25 },
    { range: "100-500", count: 78, percentage: 43 },
    { range: "500-1000", count: 32, percentage: 18 },
    { range: "1000-5000", count: 18, percentage: 10 },
    { range: "5000+", count: 7, percentage: 4 },
  ]
}

const generateOrderTypeData = () => [
  { name: "Market Orders", value: 35, color: "#3b82f6" },
  { name: "Limit Orders", value: 65, color: "#10b981" },
]

const generateTradingActivityData = () => {
  const hours = []
  for (let i = 0; i < 24; i++) {
    hours.push({
      hour: i,
      trades: Math.floor(Math.random() * 50) + 10,
      volume: Math.floor(Math.random() * 5000) + 1000,
    })
  }
  return hours
}

const generateSpreadData = () => {
  const data = []
  for (let i = 0; i < 20; i++) {
    data.push({
      time: `${9 + Math.floor(i / 2)}:${i % 2 === 0 ? "00" : "30"}`,
      spread: Number((Math.random() * 0.5 + 0.1).toFixed(3)),
      bidAskRatio: Number((Math.random() * 2 + 0.5).toFixed(2)),
    })
  }
  return data
}

export default function StatisticsPage() {
  const [stats, setStats] = useState<Statistics | null>(null)
  const [loading, setLoading] = useState(true)
  const [timeRange, setTimeRange] = useState("24h")
  const [priceData] = useState(generatePriceData())
  const [volumeData] = useState(generateVolumeData())
  const [orderTypeData] = useState(generateOrderTypeData())
  const [activityData] = useState(generateTradingActivityData())
  const [spreadData] = useState(generateSpreadData())

  useEffect(() => {
    const fetchStatistics = async () => {
      try {
        const response = await fetch("http://localhost:18080/statistics")
        if (response.ok) {
          const data = await response.json()
          setStats(data)
        }
      } catch (error) {
        console.error("Failed to fetch statistics:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchStatistics()
    const interval = setInterval(fetchStatistics, 10000)
    return () => clearInterval(interval)
  }, [])

  const CustomTooltip = ({ active, payload, label }: any) => {
    if (active && payload && payload.length) {
      return (
        <div className="bg-white border border-gray-200 rounded-lg p-3 shadow-lg">
          <p className="text-gray-600 text-sm">{`Time: ${label}`}</p>
          {payload.map((entry: any, index: number) => (
            <p key={index} style={{ color: entry.color }} className="font-semibold">
              {`${entry.dataKey}: ${entry.value}`}
            </p>
          ))}
        </div>
      )
    }
    return null
  }

  if (loading) {
    return (
      <div className="space-y-6">
        <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-2xl font-semibold text-gray-900">Trading Analytics</h1>
              <p className="text-gray-600 mt-1">Comprehensive statistics and performance metrics</p>
            </div>
            <Badge className="bg-blue-50 text-blue-700 border-blue-200 px-4 py-2">Auto-refresh every 10s</Badge>
          </div>
        </motion.div>

        <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-4">
          {[...Array(4)].map((_, i) => (
            <Card key={i} className="border-gray-200">
              <CardHeader className="pb-2">
                <div className="h-4 w-20 bg-gray-200 animate-pulse rounded" />
              </CardHeader>
              <CardContent>
                <div className="h-8 w-24 bg-gray-200 animate-pulse rounded mb-2" />
                <div className="h-3 w-32 bg-gray-200 animate-pulse rounded" />
              </CardContent>
            </Card>
          ))}
        </div>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <motion.div initial={{ opacity: 0, y: -20 }} animate={{ opacity: 1, y: 0 }}>
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-semibold text-gray-900">Trading Analytics</h1>
            <p className="text-gray-600 mt-1">Comprehensive statistics and performance metrics</p>
          </div>
          <div className="flex items-center gap-4">
            <Select value={timeRange} onValueChange={setTimeRange}>
              <SelectTrigger className="w-32 border-gray-200 focus:border-blue-300">
                <SelectValue />
              </SelectTrigger>
              <SelectContent>
                <SelectItem value="1h">Last Hour</SelectItem>
                <SelectItem value="24h">Last 24h</SelectItem>
                <SelectItem value="7d">Last 7 Days</SelectItem>
                <SelectItem value="30d">Last 30 Days</SelectItem>
              </SelectContent>
            </Select>
            <Badge className="bg-blue-50 text-blue-700 border-blue-200 px-4 py-2">Auto-refresh every 10s</Badge>
          </div>
        </div>
      </motion.div>

      {/* Key Metrics Cards */}
      <motion.div
        initial={{ opacity: 0, y: 20 }}
        animate={{ opacity: 1, y: 0 }}
        className="grid gap-6 md:grid-cols-2 lg:grid-cols-4 mb-8"
      >
        <Card className="border-gray-200">
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium text-gray-600">Total Trades</CardTitle>
            <Activity className="h-4 w-4 text-blue-600" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-semibold text-blue-600">{stats?.trades.total_trades || 0}</div>
            <p className="text-xs text-gray-500">Executed trades</p>
          </CardContent>
        </Card>

        <Card className="border-gray-200">
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium text-gray-600">Total Volume</CardTitle>
            <BarChart3 className="h-4 w-4 text-green-600" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-semibold text-green-600">
              {stats?.trades.total_volume?.toLocaleString() || 0}
            </div>
            <p className="text-xs text-gray-500">Shares traded</p>
          </CardContent>
        </Card>

        <Card className="border-gray-200">
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium text-gray-600">Total Value</CardTitle>
            <DollarSign className="h-4 w-4 text-purple-600" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-semibold text-purple-600">
              $
              {stats?.trades.total_value?.toLocaleString(undefined, {
                minimumFractionDigits: 2,
                maximumFractionDigits: 2,
              }) || "0.00"}
            </div>
            <p className="text-xs text-gray-500">Dollar volume</p>
          </CardContent>
        </Card>

        <Card className="border-gray-200">
          <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-2">
            <CardTitle className="text-sm font-medium text-gray-600">Average Price</CardTitle>
            <TrendingUp className="h-4 w-4 text-orange-600" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-semibold text-orange-600">
              ${stats?.trades.avg_price?.toFixed(2) || "0.00"}
            </div>
            <p className="text-xs text-gray-500">Volume weighted</p>
          </CardContent>
        </Card>
      </motion.div>

      {/* Price and Volume Charts */}
      <div className="grid gap-6 lg:grid-cols-2">
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <LineChart className="h-5 w-5 text-blue-600" />
                Price Movement ({timeRange})
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <AreaChart data={priceData}>
                    <defs>
                      <linearGradient id="priceGradient" x1="0" y1="0" x2="0" y2="1">
                        <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3} />
                        <stop offset="95%" stopColor="#3b82f6" stopOpacity={0} />
                      </linearGradient>
                    </defs>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                    <XAxis dataKey="time" stroke="#6b7280" fontSize={12} />
                    <YAxis stroke="#6b7280" fontSize={12} />
                    <Tooltip content={<CustomTooltip />} />
                    <Area type="monotone" dataKey="price" stroke="#3b82f6" strokeWidth={2} fill="url(#priceGradient)" />
                  </AreaChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </motion.div>

        <motion.div initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <BarChart3 className="h-5 w-5 text-green-600" />
                Volume Distribution
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <RechartsBarChart data={volumeData}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                    <XAxis dataKey="range" stroke="#6b7280" fontSize={12} />
                    <YAxis stroke="#6b7280" fontSize={12} />
                    <Tooltip content={<CustomTooltip />} />
                    <Bar dataKey="count" fill="#10b981" radius={[4, 4, 0, 0]} />
                  </RechartsBarChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </motion.div>
      </div>

      {/* Trading Activity and Order Types */}
      <div className="grid gap-6 lg:grid-cols-3">
        <motion.div initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }} className="lg:col-span-2">
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <Activity className="h-5 w-5 text-purple-600" />
                Trading Activity Heatmap
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <ComposedChart data={activityData}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                    <XAxis dataKey="hour" stroke="#6b7280" fontSize={12} />
                    <YAxis yAxisId="left" stroke="#6b7280" fontSize={12} />
                    <YAxis yAxisId="right" orientation="right" stroke="#6b7280" fontSize={12} />
                    <Tooltip content={<CustomTooltip />} />
                    <Bar yAxisId="left" dataKey="trades" fill="#8b5cf6" radius={[2, 2, 0, 0]} />
                    <Line
                      yAxisId="right"
                      type="monotone"
                      dataKey="volume"
                      stroke="#f59e0b"
                      strokeWidth={2}
                      dot={{ r: 4 }}
                    />
                  </ComposedChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </motion.div>

        <motion.div initial={{ opacity: 0, y: 20 }} animate={{ opacity: 1, y: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <PieChart className="h-5 w-5 text-indigo-600" />
                Order Types
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <RechartsPieChart>
                    <Pie
                      data={orderTypeData}
                      cx="50%"
                      cy="50%"
                      innerRadius={60}
                      outerRadius={120}
                      paddingAngle={5}
                      dataKey="value"
                    >
                      {orderTypeData.map((entry, index) => (
                        <Cell key={`cell-${index}`} fill={entry.color} />
                      ))}
                    </Pie>
                    <Tooltip />
                  </RechartsPieChart>
                </ResponsiveContainer>
              </div>
              <div className="mt-4 space-y-2">
                {orderTypeData.map((item, index) => (
                  <div key={index} className="flex items-center justify-between">
                    <div className="flex items-center gap-2">
                      <div className="w-3 h-3 rounded-full" style={{ backgroundColor: item.color }} />
                      <span className="text-sm text-gray-600">{item.name}</span>
                    </div>
                    <span className="text-sm font-medium text-gray-900">{item.value}%</span>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        </motion.div>
      </div>

      {/* Spread Analysis and Price vs Volume */}
      <div className="grid gap-6 lg:grid-cols-2">
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <TrendingUp className="h-5 w-5 text-red-600" />
                Spread Analysis
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <RechartsLineChart data={spreadData}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                    <XAxis dataKey="time" stroke="#6b7280" fontSize={12} />
                    <YAxis stroke="#6b7280" fontSize={12} />
                    <Tooltip content={<CustomTooltip />} />
                    <Line
                      type="monotone"
                      dataKey="spread"
                      stroke="#ef4444"
                      strokeWidth={2}
                      dot={{ r: 4, fill: "#ef4444" }}
                    />
                  </RechartsLineChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </motion.div>

        <motion.div initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <BarChart3 className="h-5 w-5 text-cyan-600" />
                Price vs Volume Correlation
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="h-80">
                <ResponsiveContainer width="100%" height="100%">
                  <ScatterChart data={priceData}>
                    <CartesianGrid strokeDasharray="3 3" stroke="#e5e7eb" />
                    <XAxis dataKey="price" stroke="#6b7280" fontSize={12} name="Price" />
                    <YAxis dataKey="volume" stroke="#6b7280" fontSize={12} name="Volume" />
                    <Tooltip
                      cursor={{ strokeDasharray: "3 3" }}
                      content={({ active, payload }) => {
                        if (active && payload && payload.length) {
                          return (
                            <div className="bg-white border border-gray-200 rounded-lg p-3 shadow-lg">
                              <p className="text-gray-600 text-sm">Price: ${payload[0]?.payload?.price}</p>
                              <p className="text-gray-600 text-sm">Volume: {payload[0]?.payload?.volume}</p>
                            </div>
                          )
                        }
                        return null
                      }}
                    />
                    <Scatter dataKey="volume" fill="#06b6d4" />
                  </ScatterChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </motion.div>
      </div>

      {/* Order Book and Market Data Stats */}
      <div className="grid gap-6 md:grid-cols-2">
        <motion.div initial={{ opacity: 0, x: -20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <BarChart3 className="h-5 w-5 text-blue-600" />
                Order Book Statistics
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="grid gap-4 md:grid-cols-2">
                <div className="space-y-4">
                  <div>
                    <div className="text-2xl font-semibold text-green-600">{stats?.orderbook.bid_levels || 0}</div>
                    <div className="text-sm text-gray-500">Bid Levels</div>
                  </div>
                  <div>
                    <div className="text-2xl font-semibold text-green-600">{stats?.orderbook.bid_orders || 0}</div>
                    <div className="text-sm text-gray-500">Bid Orders</div>
                  </div>
                </div>
                <div className="space-y-4">
                  <div>
                    <div className="text-2xl font-semibold text-red-600">{stats?.orderbook.ask_levels || 0}</div>
                    <div className="text-sm text-gray-500">Ask Levels</div>
                  </div>
                  <div>
                    <div className="text-2xl font-semibold text-red-600">{stats?.orderbook.ask_orders || 0}</div>
                    <div className="text-sm text-gray-500">Ask Orders</div>
                  </div>
                </div>
              </div>
              <div className="my-4 h-[1px] bg-gray-200" />
              <div className="flex justify-between items-center">
                <span className="text-sm text-gray-500">Total Active Orders</span>
                <span className="text-lg font-semibold text-gray-900">{stats?.orderbook.total_orders || 0}</span>
              </div>
            </CardContent>
          </Card>
        </motion.div>

        <motion.div initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }}>
          <Card className="border-gray-200">
            <CardHeader>
              <CardTitle className="flex items-center gap-2 text-gray-900">
                <Activity className="h-5 w-5 text-blue-600" />
                Price Statistics
              </CardTitle>
            </CardHeader>
            <CardContent>
              <div className="grid gap-6 md:grid-cols-3">
                <div className="text-center">
                  <div className="text-2xl font-semibold text-red-600">
                    ${stats?.trades.min_price?.toFixed(2) || "0.00"}
                  </div>
                  <div className="text-sm text-gray-500">Minimum Price</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-semibold text-blue-600">
                    ${stats?.trades.avg_price?.toFixed(2) || "0.00"}
                  </div>
                  <div className="text-sm text-gray-500">Average Price</div>
                </div>
                <div className="text-center">
                  <div className="text-2xl font-semibold text-green-600">
                    ${stats?.trades.max_price?.toFixed(2) || "0.00"}
                  </div>
                  <div className="text-sm text-gray-500">Maximum Price</div>
                </div>
              </div>
              {stats?.trades.last_trade_time && (
                <>
                  <div className="my-4 h-[1px] bg-gray-200" />
                  <div className="text-center">
                    <div className="text-sm text-gray-500">Last Trade</div>
                    <div className="text-lg font-medium text-gray-900">
                      {new Date(stats.trades.last_trade_time).toLocaleString()}
                    </div>
                  </div>
                </>
              )}
            </CardContent>
          </Card>
        </motion.div>
      </div>
    </div>
  )
}
