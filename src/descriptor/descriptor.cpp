#include "descriptor.hpp"
 
// std
#include <cassert>
#include <stdexcept>
 
namespace nugiEngine {
  // *************** Descriptor Set Layout Builder *********************
  
  EngineDescriptorSetLayout::Builder &EngineDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) 
  {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
  }
  
  std::shared_ptr<EngineDescriptorSetLayout> EngineDescriptorSetLayout::Builder::build() const {
    return std::make_shared<EngineDescriptorSetLayout>(this->engineDevice, bindings);
  }
  
  // *************** Descriptor Set Layout *********************
  
  EngineDescriptorSetLayout::EngineDescriptorSetLayout(
      EngineDevice &engineDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
      : engineDevice{engineDevice}, bindings{bindings} {
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto& kv : bindings) {
      setLayoutBindings.push_back(kv.second);
    }
  
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
  
    if (vkCreateDescriptorSetLayout(
      this->engineDevice.getLogicalDevice(),
      &descriptorSetLayoutInfo,
      nullptr,
      &this->descriptorSetLayout) != VK_SUCCESS) 
    {
      throw std::runtime_error("failed to create descriptor set layout!");
    }
  }
  
  EngineDescriptorSetLayout::~EngineDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(this->engineDevice.getLogicalDevice(), descriptorSetLayout, nullptr);
  }
  
  // *************** Descriptor Pool Builder *********************
  
  EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::addPoolSize(
      VkDescriptorType descriptorType, uint32_t count) {
    this->poolSizes.push_back({descriptorType, count});
    return *this;
  }
  
  EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::setPoolFlags(
      VkDescriptorPoolCreateFlags flags) {
    this->poolFlags = flags;
    return *this;
  }
  EngineDescriptorPool::Builder &EngineDescriptorPool::Builder::setMaxSets(uint32_t count) {
    this->maxSets = count;
    return *this;
  }
  
  std::shared_ptr<EngineDescriptorPool> EngineDescriptorPool::Builder::build() const {
    return std::make_shared<EngineDescriptorPool>(this->engineDevice, this->maxSets, this->poolFlags, this->poolSizes);
  }
  
  // *************** Descriptor Pool *********************
  
  EngineDescriptorPool::EngineDescriptorPool(
      EngineDevice &engineDevice,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes)
      : engineDevice{engineDevice} 
  {
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.flags = poolFlags;
  
    if (vkCreateDescriptorPool(this->engineDevice.getLogicalDevice(), &descriptorPoolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS) {
      throw std::runtime_error("failed to create descriptor pool!");
    }
  }
  
  EngineDescriptorPool::~EngineDescriptorPool() {
    vkDestroyDescriptorPool(this->engineDevice.getLogicalDevice(), descriptorPool, nullptr);
  }
  
  bool EngineDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet *descriptor) const {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
  
    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool whenever an old pool fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(this->engineDevice.getLogicalDevice(), &allocInfo, descriptor) != VK_SUCCESS) {
      return false;
    }
    return true;
  }
  
  void EngineDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
      this->engineDevice.getLogicalDevice(),
      this->descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data()
    );
  }
  
  void EngineDescriptorPool::resetPool() {
    vkResetDescriptorPool(this->engineDevice.getLogicalDevice(), this->descriptorPool, 0);
  }
  
  // *************** Descriptor Writer *********************
  
  EngineDescriptorWriter::EngineDescriptorWriter(EngineDescriptorSetLayout &setLayout, EngineDescriptorPool &pool) : setLayout{setLayout}, pool{pool} {}
  
  EngineDescriptorWriter &EngineDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo, uint32_t count) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
  
    auto &bindingDescription = setLayout.bindings[binding];
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = count;
  
    writes.push_back(write);
    return *this;
  }
  
  EngineDescriptorWriter &EngineDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo, uint32_t count) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
  
    auto &bindingDescription = setLayout.bindings[binding];
    
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = count;
  
    writes.push_back(write);
    return *this;
  }
  
  bool EngineDescriptorWriter::build(VkDescriptorSet *set) {
    bool success = this->pool.allocateDescriptor(this->setLayout.getDescriptorSetLayout(), set);
    if (!success) {
      return false;
    }

    this->overwrite(set);
    return true;
  }
  
  void EngineDescriptorWriter::overwrite(VkDescriptorSet *set) {
    for (auto &write : this->writes) {
      write.dstSet = *set;
    }
    
    vkUpdateDescriptorSets(this->pool.engineDevice.getLogicalDevice(), static_cast<uint32_t>(this->writes.size()), this->writes.data(), 0, nullptr);
  }
 
}  // namespace lve